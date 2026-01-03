// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_WROVER_KIT

// Includes
#include "esp_camera.h"
#include "moteur/MotorPan.h"
#include "moteur/MotorTilt.h"
#include <WiFi.h>
#include "camera/camera_pins.h"

// Init wifi
const char *ssid_Router = WIFI_SSID;
const char *password_Router = WIFI_PASSWORD;
camera_config_t config;

// Variable pour la capture du moment de demarrage des moteurs
unsigned long startTimePan = 0;
unsigned long startTimeTilt = 0;

// Axe vertical (haut / bas)
MotorTilt motorTilt(25, 26);

// Axe horizontal (gauche / droite)
MotorPan motorPan(27, 14);

// Durée de marche moteur
const unsigned long MOTOR_TIME = 50; // en ms

void startCameraServer();
void camera_init();

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_init();

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid_Router, password_Router);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop()
{
  // Variable pour l'etat du moteur horizontal
  MotorPan::MotorState panState = motorPan.getState();
  // Variable pour l'etat du moteur vertical
  MotorTilt::MotorState tiltState = motorTilt.getState();
  if (panState != MotorPan::STOP)
  {
    if (millis() - startTimePan >= MOTOR_TIME)
    {
      motorPan.stop();
    }
  }
  if (tiltState != MotorTilt::STOP)
  {
    if (millis() - startTimeTilt >= MOTOR_TIME)
    {
      motorTilt.stop();
    }
  }
}

void camera_init()
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
}
