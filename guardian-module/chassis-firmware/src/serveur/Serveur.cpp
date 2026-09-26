#include "esp_http_server.h"
#include <Arduino.h>
#include "Serveur.h"
#include "Config.h"
#include "../moteur/MecanumChassis.h"

extern MecanumChassis chassis;

static const char *TAG = "chassis_httpd";

/* ============================
   UTILS
   ============================ */

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
   HANDLER
   ============================ */

// /control?cmd=forward&speed=200
static esp_err_t cmd_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char cmd[32];
    char speed_str[16];

    if (parse_get(req, &buf) != ESP_OK)
        return httpd_resp_send_404(req);

    if (httpd_query_key_value(buf, "cmd", cmd, sizeof(cmd)) != ESP_OK)
    {
        free(buf);
        return httpd_resp_send_404(req);
    }

    // speed est optionnel, défaut 200
    int speed = 255;
    if (httpd_query_key_value(buf, "speed", speed_str, sizeof(speed_str)) == ESP_OK)
        speed = atoi(speed_str);

    free(buf);

    // Clamp speed
    speed = constrain(speed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);

    if (!strcmp(cmd, "forward"))
        chassis.forward(speed);
    else if (!strcmp(cmd, "backward"))
        chassis.backward(speed);
    else if (!strcmp(cmd, "strafe_left"))
        chassis.strafeLeft(speed);
    else if (!strcmp(cmd, "strafe_right"))
        chassis.strafeRight(speed);
    else if (!strcmp(cmd, "rotate_left"))
        chassis.rotateLeft(speed);
    else if (!strcmp(cmd, "rotate_right"))
        chassis.rotateRight(speed);
    else if (!strcmp(cmd, "stop"))
        chassis.stop();
    else
        ESP_LOGI(TAG, "Unknown command: %s", cmd);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

/* ============================
   SERVER START
   ============================ */

httpd_handle_t chassis_httpd = NULL;

void startChassisServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_PORT;

    httpd_uri_t control_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler};

    ESP_LOGI(TAG, "Starting chassis server on port %d", HTTP_PORT);
    httpd_start(&chassis_httpd, &config);
    httpd_register_uri_handler(chassis_httpd, &control_uri);
}