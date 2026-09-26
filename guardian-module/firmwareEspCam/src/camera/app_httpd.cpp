// ============================================================================
// Serveur HTTP pour la caméra ESP32
// Gère le flux vidéo MJPEG sur le port 81
// ============================================================================

#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "Arduino.h"

static const char *TAG = "camera_httpd";

// Un seul stream vidéo à la fois
static volatile bool stream_running = false;

// ============================================================================
// STREAM MJPEG
// ============================================================================

#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

static const char *_STREAM_BOUNDARY =
    "\r\n--" PART_BOUNDARY "\r\n";

static const char *_STREAM_PART =
    "Content-Type: image/jpeg\r\n"
    "Content-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

// ============================================================================
// STREAM HANDLER
// ============================================================================

static esp_err_t stream_handler(httpd_req_t *req)
{
    // Empêche plusieurs clients d'ouvrir le stream simultanément
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
        // Capture d'une image JPEG
        camera_fb_t *fb = esp_camera_fb_get();

        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            break;
        }

        // --------------------------------------------------------------------
        // Boundary
        // --------------------------------------------------------------------

        if (httpd_resp_send_chunk(
                req,
                _STREAM_BOUNDARY,
                strlen(_STREAM_BOUNDARY)) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }

        // --------------------------------------------------------------------
        // Header JPEG
        // --------------------------------------------------------------------

        char header[64];

        size_t hlen = snprintf(
            header,
            sizeof(header),
            _STREAM_PART,
            fb->len);

        // --------------------------------------------------------------------
        // Image JPEG
        // --------------------------------------------------------------------

        if (httpd_resp_send_chunk(req, header, hlen) != ESP_OK ||
            httpd_resp_send_chunk(
                req,
                (const char *)fb->buf,
                fb->len) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }

        // Libération du buffer caméra
        esp_camera_fb_return(fb);

        // Laisse un peu de temps aux autres tâches ESP32
        delay(1);
    }

    // Libération du stream
    stream_running = false;

    // Termine proprement la réponse HTTP
    httpd_resp_send_chunk(req, NULL, 0);

    ESP_LOGI(TAG, "Stream stopped");

    return ESP_OK;
}

// ============================================================================
// DÉMARRAGE DU SERVEUR
// ============================================================================

void startCameraServer()
{
    // ------------------------------------------------------------------------
    // Serveur du flux vidéo
    // ------------------------------------------------------------------------

    httpd_config_t stream_config = HTTPD_DEFAULT_CONFIG();

    stream_config.server_port = 81;
    stream_config.ctrl_port = 32769;

    // ------------------------------------------------------------------------
    // Route /stream
    // ------------------------------------------------------------------------

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL};

    // ------------------------------------------------------------------------
    // Démarrage
    // ------------------------------------------------------------------------

    ESP_LOGI(TAG, "Starting stream server on port 81");

    esp_err_t err = httpd_start(
        &stream_httpd,
        &stream_config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start stream server: %s",
                 esp_err_to_name(err));
        return;
    }

    err = httpd_register_uri_handler(
        stream_httpd,
        &stream_uri);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register /stream: %s",
                 esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Stream ready: /stream");
}
