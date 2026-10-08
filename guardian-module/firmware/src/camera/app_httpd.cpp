// ============================================================================
// Serveur HTTP pour la caméra ESP32
// - sert la page web de contrôle
// - reçoit les commandes envoyées par le navigateur
// ============================================================================

#include "esp_http_server.h"

#include "camera_index.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <Arduino.h>
#include "moteur/Motor.h"
#include "laser/Laser.h"
#include "../Config.h"

// Accès aux moteurs déclarés dans main.cpp
// Permet de contrôler la tourelle depuis les requêtes HTTP
extern Motor motorCirculaire;
extern Motor motorElevation;
extern Motor motorFire;
extern Laser laser;

static const char *TAG = "camera_httpd";

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

// Applique une commande, qu'elle vienne de /control (HTTP) ou de /ws (WebSocket)
static void apply_command(const char *var, int val)
{
    // Commandes de la tourelle
    // pan  -> rotation horizontale

    if (!strcmp(var, "pan"))
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
    } // tilt -> rotation verticale
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
    } // Fire -> cycle de tir
    else if (!strcmp(var, "Fire"))
    {
        if (val == 1)
        {
            motorFire.startFireCycle(config::FIRE_CYCLE);
        }
    }
    else if (!strcmp(var, "laser"))
    {
        if (val == 1)
        {
            laser.on();
        }
        else if (val == 0)
        {
            laser.off();
        }
    }
    else
        ESP_LOGI(TAG, "Unknown command: %s", var);
}

// Reçoit les commandes envoyées par la page web
// exemple : /control?var=pan&val=1
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

    apply_command(var, atoi(val_str));

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

/* ============================
   WEBSOCKET
   ============================ */

// ---------------------------------------------------------------------------
// Liaison WebSocket /ws
// Messages texte "<var> <val>", mêmes commandes que /control :
//   "pan 1", "tilt 0", "Fire 1", "laser 1"
// "ping 0" -> répond "pong" (mesure de latence + battement de cœur).
// Une seule connexion TCP : les commandes arrivent dans l'ordre d'envoi.
//
// Sécurité : si la liaison se ferme, ou si plus aucun message n'arrive
// pendant WS_TIMEOUT_MS, la tourelle s'arrête (voir ws_watchdog()).
// ---------------------------------------------------------------------------

static volatile int ws_fd = -1;                // client WebSocket courant
static volatile unsigned long ws_last_msg = 0; // millis() du dernier message
static volatile bool ws_armed = false;         // un client est connecté et surveillé

static void stop_turret()
{
    motorCirculaire.stop();
    motorElevation.stop();
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    // Premier appel = poignée de main HTTP -> WebSocket
    if (req->method == HTTP_GET)
    {
        ws_fd = httpd_req_to_sockfd(req);
        ws_last_msg = millis();
        ws_armed = true;
        ESP_LOGI(TAG, "WebSocket connected (fd %d)", ws_fd);
        return ESP_OK;
    }

    httpd_ws_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    // Lecture de la taille, puis du contenu
    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK)
        return ret;

    uint8_t buf[64];
    if (frame.len >= sizeof(buf))
        return ESP_FAIL; // message trop long : on coupe la connexion

    frame.payload = buf;
    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK)
        return ret;
    buf[frame.len] = '\0';

    if (frame.type != HTTPD_WS_TYPE_TEXT)
        return ESP_OK;

    ws_fd = httpd_req_to_sockfd(req);
    ws_last_msg = millis();
    ws_armed = true;

    char var[32];
    int val = 0;
    if (sscanf((const char *)buf, "%31s %d", var, &val) < 1)
        return ESP_OK;

    if (!strcmp(var, "ping"))
    {
        httpd_ws_frame_t pong;
        memset(&pong, 0, sizeof(pong));
        pong.type = HTTPD_WS_TYPE_TEXT;
        pong.payload = (uint8_t *)"pong";
        pong.len = 4;
        return httpd_ws_send_frame(req, &pong);
    }

    apply_command(var, val);
    return ESP_OK;
}

// Appelé par le serveur à la fermeture de n'importe quelle socket.
// Si c'est le client WebSocket : arrêt immédiat de la tourelle.
static void on_socket_close(httpd_handle_t hd, int sockfd)
{
    if (sockfd == ws_fd)
    {
        ESP_LOGI(TAG, "WebSocket closed (fd %d) -> turret stop", sockfd);
        ws_fd = -1;
        ws_armed = false;
        stop_turret();
    }
    close(sockfd); // obligatoire quand close_fn est défini
}

// À appeler dans loop() : coupe la tourelle si la page ne donne plus signe de vie
// (WiFi perdu sans fermeture propre de la connexion).
void ws_watchdog()
{
    if (ws_armed && millis() - ws_last_msg > config::WS_TIMEOUT_MS)
    {
        ESP_LOGI(TAG, "WebSocket silent -> turret stop");
        ws_armed = false; // une seule fois ; réarmé au prochain message
        stop_turret();
    }
}

/* ============================
   SERVER START
   ============================ */

// ---------------------------------------------------------------------------
// Démarrage du serveur HTTP
// ---------------------------------------------------------------------------

// Initialise le serveur HTTP :
// port 80 -> page web + commandes
httpd_handle_t camera_httpd = NULL;

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = config::HTTP_PORT;
    config.close_fn = on_socket_close;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler};

    httpd_uri_t control_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler};

    httpd_uri_t ws_uri = {};
    ws_uri.uri = "/ws";
    ws_uri.method = HTTP_GET;
    ws_uri.handler = ws_handler;
    ws_uri.is_websocket = true;

    ESP_LOGI(TAG, "Starting control server on port 80");
    httpd_start(&camera_httpd, &config);

    httpd_register_uri_handler(camera_httpd, &index_uri);

    httpd_register_uri_handler(camera_httpd, &control_uri);

    httpd_register_uri_handler(camera_httpd, &ws_uri);
}
