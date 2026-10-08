// ============================================================================
// Serveur HTTP pour la caméra ESP32 (port 81)
//  - /ws     : vidéo par WebSocket, une image JPEG par demande (utilisé par la page)
//  - /stream : flux MJPEG classique (secours / debug)
// ============================================================================

#include "Arduino.h" // avant lwip : sinon conflit sur INADDR_NONE (IPAddress.h)
#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "lwip/sockets.h"

static const char *TAG = "camera_httpd";

// Un seul stream vidéo à la fois
static volatile bool stream_running = false;

// Désactive l'algorithme de Nagle sur la socket : sans ça, les petits envois
// (en-têtes) attendent l'accusé de réception du PC, qui lui-même retarde ses
// accusés → jusqu'à ~200 ms perdues par image.
static void set_no_delay(httpd_req_t *req)
{
    int one = 1;
    setsockopt(httpd_req_to_sockfd(req), IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
}

// ============================================================================
// STREAM MJPEG
// ============================================================================

#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

static const char *_STREAM_BOUNDARY =
    "\r\n--" PART_BOUNDARY "\r\n";

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
    set_no_delay(req);

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
        // Boundary + header JPEG, envoyés en un seul morceau
        // --------------------------------------------------------------------

        char header[128];

        size_t hlen = snprintf(
            header,
            sizeof(header),
            "%s" "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
            _STREAM_BOUNDARY,
            fb->len);

        // --------------------------------------------------------------------
        // Image JPEG
        // --------------------------------------------------------------------

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
// VIDÉO PAR WEBSOCKET
// ----------------------------------------------------------------------------
// Fonctionnement "à la demande" : chaque message texte "next" reçu de la page
// déclenche l'envoi d'UNE image JPEG (message binaire), la plus récente
// disponible. La caméra n'envoie jamais plus que ce que la page a demandé :
// aucune image ne peut s'empiler dans le réseau, le retard reste borné même
// quand le WiFi faiblit. La page garde 2 demandes en vol pour la fluidité.
// ============================================================================

static esp_err_t ws_video_handler(httpd_req_t *req)
{
    // Premier appel = poignée de main HTTP -> WebSocket
    if (req->method == HTTP_GET)
    {
        set_no_delay(req);
        ESP_LOGI(TAG, "Video WebSocket connected");
        return ESP_OK;
    }

    httpd_ws_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK)
        return ret;

    uint8_t buf[16];
    if (frame.len >= sizeof(buf))
        return ESP_FAIL; // message inattendu : on coupe la connexion

    frame.payload = buf;
    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK)
        return ret;
    buf[frame.len] = '\0';

    if (frame.type != HTTPD_WS_TYPE_TEXT || strcmp((const char *)buf, "next") != 0)
        return ESP_OK;

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    httpd_ws_frame_t out;
    memset(&out, 0, sizeof(out));
    out.type = HTTPD_WS_TYPE_BINARY;
    out.payload = fb->buf;
    out.len = fb->len;
    ret = httpd_ws_send_frame(req, &out);

    esp_camera_fb_return(fb);
    return ret;
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
    // Route /ws (vidéo WebSocket)
    // ------------------------------------------------------------------------

    httpd_uri_t ws_uri = {};
    ws_uri.uri = "/ws";
    ws_uri.method = HTTP_GET;
    ws_uri.handler = ws_video_handler;
    ws_uri.is_websocket = true;

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

    err = httpd_register_uri_handler(
        stream_httpd,
        &ws_uri);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register /ws: %s",
                 esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Stream ready: /ws (WebSocket), /stream (MJPEG)");
}
