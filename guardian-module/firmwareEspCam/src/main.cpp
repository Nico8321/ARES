// ============================================================================
// ARES - ESP32-CAM
// Connexion Wi-Fi + configuration caméra
// ============================================================================

#include "esp_camera.h"
#include <WiFi.h>
#include <ESPmDNS.h>

// ============================================================================
// MODÈLE DE CAMÉRA
// ============================================================================

#define CAMERA_MODEL_AI_THINKER

#include "camera/camera_pins.h"

// ============================================================================
// WI-FI
// ============================================================================

const char *ssid_Router = "ARES";
const char *password_Router = "12345678";

// ============================================================================
// CONFIGURATION CAMÉRA
// ============================================================================

camera_config_t config;

void startCameraServer();
void camera_init();

// ============================================================================
// SETUP
// ============================================================================

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // ------------------------------------------------------------------------
  // Initialisation caméra
  // ------------------------------------------------------------------------

  camera_init();

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK)
  {
    Serial.printf(
        "Camera init failed with error 0x%x\n",
        err);

    return;
  }

  // ------------------------------------------------------------------------
  // Réglages image
  // ------------------------------------------------------------------------

  sensor_t *s = esp_camera_sensor_get();

  s->set_vflip(s, 0);       // 0 = normal
  s->set_hmirror(s, 1);     // 1 = miroir horizontal
  s->set_brightness(s, 1);  // légère augmentation luminosité
  s->set_saturation(s, -1); // saturation légèrement réduite

  // ------------------------------------------------------------------------
  // Connexion Wi-Fi
  // ------------------------------------------------------------------------

  WiFi.begin(ssid_Router, password_Router);

  // Désactive le Wi-Fi Sleep pour réduire la latence
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  // ------------------------------------------------------------------------
  // mDNS
  // ------------------------------------------------------------------------

  if (!MDNS.begin("arescam"))
  {
    Serial.println("Erreur mDNS");
  }
  else
  {
    Serial.println("mDNS démarré : http://arescam.local");
  }

  // ------------------------------------------------------------------------
  // Serveur caméra
  // ------------------------------------------------------------------------

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("'");
}

// ============================================================================
// LOOP
// ============================================================================

void loop()
{
  // Rien à exécuter ici.
}

// ============================================================================
// CONFIGURATION CAMÉRA
// ============================================================================

void camera_init()
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  // ------------------------------------------------------------------------
  // Pins caméra
  // ------------------------------------------------------------------------

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

  // ------------------------------------------------------------------------
  // Horloge caméra
  // ------------------------------------------------------------------------

  config.xclk_freq_hz = 20000000;

  // ------------------------------------------------------------------------
  // Format vidéo
  // ------------------------------------------------------------------------

  config.pixel_format = PIXFORMAT_JPEG;

  // VGA = 640x480
  // Plus léger que HD → moins de données à transmettre
  config.frame_size = FRAMESIZE_VGA;

  // JPEG :
  // plus petit = meilleure qualité / fichier plus lourd
  // plus grand = compression plus forte / fichier plus léger
  config.jpeg_quality = 12;

  // ------------------------------------------------------------------------
  // Gestion des frames
  // ------------------------------------------------------------------------

  // Toujours privilégier l'image la plus récente
  // Important pour réduire la latence du pilotage.
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Utilisation de la PSRAM
  config.fb_location = CAMERA_FB_IN_PSRAM;

  // Deux buffers suffisent pour notre utilisation
  config.fb_count = 2;
}
