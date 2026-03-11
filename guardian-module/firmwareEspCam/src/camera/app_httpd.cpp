// ============================================================================
// Serveur HTTP pour la caméra ESP32

// - gère le stream vidéo
// ============================================================================

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"

#include "esp_log.h"
#include <Arduino.h>

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

httpd_handle_t stream_httpd = NULL;

/* ============================
   UTILS
   ============================ */

// ---------------------------------------------------------------------------
// Fonctions utilitaires
// ---------------------------------------------------------------------------

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

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler};

    ESP_LOGI(TAG, "Starting stream server on port 81");
    httpd_start(&stream_httpd, &stream_config);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
}