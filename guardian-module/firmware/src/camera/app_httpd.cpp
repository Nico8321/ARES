// ============================================================================
// Serveur HTTP pour la caméra ESP32
// - sert la page web de contrôle
// - gère le stream vidéo
// - reçoit les commandes envoyées par le navigateur
// ============================================================================

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "camera_index.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include "moteur/Motor.h"

// Accès aux moteurs déclarés dans main.cpp
// Permet de contrôler la tourelle depuis les requêtes HTTP
extern Motor motorCirculaire;
extern Motor motorElevation;
extern Motor motorFire;

static const char *TAG = "camera_httpd";
// Indique si un stream vidéo est déjà en cours
// évite d'ouvrir plusieurs flux en même temps
static volatile bool stream_running = false;
/* ============================
   STREAM DEFINITIONS
   ============================ */

// Paramètres utilisés pour le flux MJPEG
// le navigateur lit une suite d'images JPEG envoyées en continu
#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

/* ============================
   UTILS
   ============================ */

// ---------------------------------------------------------------------------
// Fonctions utilitaires
// ---------------------------------------------------------------------------

// Récupère les paramètres GET de l'URL
// exemple : /control?var=pan&val=1
static esp_err_t parse_get(httpd_req_t *req, char **obuf)
{
    size_t len = httpd_req_get_url_query_len(req) + 1;
    if (len <= 1)
        return ESP_FAIL;

    char *buf = (char *)malloc(len);
    if (!buf)
        return ESP_FAIL;

    if (httpd_req_get_url_query_str(req, buf, len) == ESP_OK)
    {
        *obuf = buf;
        return ESP_OK;
    }

    free(buf);
    return ESP_FAIL;
}

/* ============================
   HANDLERS
   ============================ */

// ---------------------------------------------------------------------------
// Handlers HTTP
// Chaque fonction correspond à une URL appelée par le navigateur
// ---------------------------------------------------------------------------

// Envoie la page web principale (interface de contrôle)
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req,
                           (const char *)index_ov3660_html_gz,
                           index_ov3660_html_gz_len);
}

// Capture une image unique JPEG
static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
        return httpd_resp_send_500(req);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_err_t res = httpd_resp_send(req,
                                    (const char *)fb->buf,
                                    fb->len);

    esp_camera_fb_return(fb);
    return res;
}

// Stream vidéo en continu (MJPEG)
// envoie des images JPEG en boucle
static esp_err_t stream_handler(httpd_req_t *req)
{
    if (stream_running)
    {
        ESP_LOGW(TAG, "Stream already running");
        httpd_resp_set_status(req, "409 Conflict");
        return httpd_resp_send(req, NULL, 0);
    }

    stream_running = true;

    httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (stream_running)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
            break;

        // Boundary
        if (httpd_resp_send_chunk(req,
                                  _STREAM_BOUNDARY,
                                  strlen(_STREAM_BOUNDARY)) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }

        // Header
        char header[64];
        size_t hlen = snprintf(header,
                               sizeof(header),
                               _STREAM_PART,
                               fb->len);

        // Payload
        if (httpd_resp_send_chunk(req, header, hlen) != ESP_OK ||
            httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }

        esp_camera_fb_return(fb);

        // Yield a little so other HTTP handlers (e.g. /control) can run.
        delay(1);
    }

    stream_running = false; // ← LIBÉRATION GARANTIE
    httpd_resp_send_chunk(req, NULL, 0);
    ESP_LOGI(TAG, "Stream stopped");
    return ESP_OK;
}

// Permet d'arrêter le stream depuis le navigateur
static esp_err_t stopstream_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Stop stream requested");
    stream_running = false;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", 2);
}

// Reçoit les commandes envoyées par la page web
// contrôle caméra et moteurs
static esp_err_t cmd_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char var[32];
    char val_str[32];

    if (parse_get(req, &buf) != ESP_OK)
        return httpd_resp_send_404(req);

    if (httpd_query_key_value(buf, "var", var, sizeof(var)) != ESP_OK ||
        httpd_query_key_value(buf, "val", val_str, sizeof(val_str)) != ESP_OK)
    {
        free(buf);
        return httpd_resp_send_404(req);
    }

    free(buf);

    int val = atoi(val_str);
    sensor_t *s = esp_camera_sensor_get();
    if (!s)
        return httpd_resp_send_500(req);

    if (!strcmp(var, "brightness"))
        s->set_brightness(s, val);
    else if (!strcmp(var, "contrast"))
        s->set_contrast(s, val);
    else if (!strcmp(var, "saturation"))
        s->set_saturation(s, val);
    else if (!strcmp(var, "quality"))
        s->set_quality(s, val);
    // Commandes de la tourelle
    // pan  -> rotation horizontale
    // tilt -> rotation verticale
    // Fire -> cycle de tir
    else if (!strcmp(var, "pan"))
    {
        if (val == -1)
        {
            motorCirculaire.up();
        }
        else if (val == 1)
        {
            motorCirculaire.down();
        }
        else
        {
            motorCirculaire.stop();
        }
    }
    else if (!strcmp(var, "tilt"))
    {
        if (val == -1)
        {
            motorElevation.down();
        }
        else if (val == 1)
        {
            motorElevation.up();
        }
        else
        {
            motorElevation.stop();
        }
    } // commande Mise de feu
    else if (!strcmp(var, "Fire"))
    {
        if (val == 1)
        {
            motorFire.startFireCycle(1100);
        }
    }
    else
        ESP_LOGI(TAG, "Unknown command: %s", var);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

/* ============================
   SERVER START
   ============================ */

// ---------------------------------------------------------------------------
// Démarrage des serveurs HTTP
// ---------------------------------------------------------------------------

// Initialise les serveurs HTTP :
// port 80 -> page web + commandes
// port 81 -> flux vidéo
void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_config_t stream_config = HTTPD_DEFAULT_CONFIG();
    stream_config.server_port = 81;
    stream_config.ctrl_port = 32769;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler};

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler};

    httpd_uri_t control_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler};

    httpd_uri_t stopstream_uri = {
        .uri = "/stopstream",
        .method = HTTP_GET,
        .handler = stopstream_handler};

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler};

    ESP_LOGI(TAG, "Starting control server on port 80");
    httpd_start(&camera_httpd, &config);

    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &control_uri);
    httpd_register_uri_handler(camera_httpd, &stopstream_uri);

    ESP_LOGI(TAG, "Starting stream server on port 81");
    httpd_start(&stream_httpd, &stream_config);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
}