// ============================================================================
//  Projet ARES
//  ESP32 + Camera + Controle moteurs
//
//  Ce fichier est le coeur du programme.
//  Il initialise :
//   - la camera
//   - le wifi (mode point d'accès autonome)
//   - le serveur web pour le stream video
//   - les moteurs de la tourelle
//   - les boutons de controle local
//
//  Ensuite la loop() tourne en permanence pour mettre à jour les moteurs
//  et lire les commandes (boutons ou commandes HTTP).
// ============================================================================
// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_WROVER_KIT

// Includes
#include "esp_camera.h"
#include "moteur/Motor.h"
#include "button/Button.h"
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <WiFi.h>
#include "camera/camera_pins.h"
#include "button/Inter.h"
#include <ESPmDNS.h>

// ---------------------------------------------------------------------------
// Déclaration des objets principaux du robot
// ---------------------------------------------------------------------------
// MCP : extension de GPIO en I2C pour lire les boutons
// Motor : classe qui gère les moteurs pas à pas
// Button : lecture simple d'un bouton
// Inter  : interrupteur pour choisir LOCAL ou REMOTE
camera_config_t config;

Adafruit_MCP23X17 mcp;
Motor motorElevation(32, 8, 9);
Motor motorCirculaire(33, 10, 11);
Motor motorFire(12, 12, 13);
Button btnFire(7);
Button btnUp(6);
Button btnDown(5);
Button btnLeft(4);
Button btnRight(3);
Inter interRemote(1);
bool lastButtonState = false;

void startCameraServer();
void camera_init();

// ---------------------------------------------------------------------------
// SETUP
// Cette fonction est exécutée une seule fois au démarrage de l'ESP32.
// Elle sert à initialiser tout le système.
// ---------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  Wire.begin(13, 14);
  mcp.begin_I2C(0x20);
  Serial.setDebugOutput(true);
  Serial.println();

  // Préparation de la configuration de la caméra
  camera_init();

  // Initialisation réelle du module caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    // return;
  }
  // Correction de l'orientation de l'image
  sensor_t *s = esp_camera_sensor_get();
  if (s)
  {
    s->set_hmirror(s, 1);
  }

  // Création d'un réseau WiFi autonome
  // Le PC ou le téléphone se connecte directement à l'ESP32
  WiFi.softAP("ARES", "12345678");

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.softAPIP());
  // Démarrage du service mDNS pour accéder au robot via http://ares.local
  if (!MDNS.begin("ares"))
  {
    Serial.println("mDNS failed");
  }
  // Démarrage du serveur HTTP qui gère :
  //  - le streaming vidéo
  //  - les commandes envoyées depuis la page web
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("' to connect");

  // Initialisation du matériel de contrôle
  // boutons physiques + moteurs
  btnUp.init();
  btnDown.init();
  btnLeft.init();
  btnRight.init();
  btnFire.init();
  interRemote.init();
  motorElevation.init();
  motorCirculaire.init();
  motorFire.init();
  motorElevation.setHold(true);
  motorFire.setStepInterval(500);
  motorElevation.setStepInterval(11000);
  lastButtonState = btnFire.isPressed();
}

// ---------------------------------------------------------------------------
// LOOP
// Cette boucle tourne en permanence.
// Elle met à jour les moteurs et lit les commandes.
// ---------------------------------------------------------------------------
void loop()
{
  // Mise à jour des moteurs
  // update() génère les impulsions STEP si le moteur est actif
  motorElevation.update();
  motorCirculaire.update();
  motorFire.update();

  // Si le robot est en mode REMOTE :
  // les commandes viennent uniquement du serveur HTTP
  if (interRemote.getMode() == Inter::REMOTE)
  {
    // seulement gérer les états
    // les pas seront générés dans update()
  }
  // Mode LOCAL : contrôle avec les boutons physiques
  else
  {
    if (btnUp.isPressed())
    {
      if (motorElevation.getState() != Motor::UP)
        motorElevation.up();
    }
    else if (btnDown.isPressed())
    {
      if (motorElevation.getState() != Motor::DOWN)
        motorElevation.down();
    }
    else
    {
      motorElevation.stop();
    }
    if (btnLeft.isPressed())
    {
      if (motorCirculaire.getState() != Motor::UP)
        motorCirculaire.up();
    }
    else if (btnRight.isPressed())
    {
      if (motorCirculaire.getState() != Motor::DOWN)
        motorCirculaire.down();
    }
    else
    {
      motorCirculaire.stop();
    }
    // Détection du tir
    // On détecte le front du bouton (appui unique)
    bool currentState = btnFire.isPressed();
    if (currentState && !lastButtonState)
      motorFire.startFireCycle(1000);

    lastButtonState = currentState;
  }
}
// ---------------------------------------------------------------------------
// Configuration de la caméra
// Cette fonction remplit la structure camera_config_t
// avec tous les paramètres nécessaires au driver ESP32.
// ---------------------------------------------------------------------------
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
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 15;
  config.fb_count = 2;
}
