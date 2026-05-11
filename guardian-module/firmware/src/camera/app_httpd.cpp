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

// Reçoit les commandes envoyées par la page web
// contrôle moteurs
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

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
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

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler};

    httpd_uri_t control_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler};

    ESP_LOGI(TAG, "Starting control server on port 80");
    httpd_start(&camera_httpd, &config);

    httpd_register_uri_handler(camera_httpd, &index_uri);

    httpd_register_uri_handler(camera_httpd, &control_uri);
}