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

// Includes

#include "moteur/Motor.h"
#include "laser/Laser.h"
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"

// ---------------------------------------------------------------------------
// Déclaration des objets principaux du robot
// ---------------------------------------------------------------------------
// MCP : extension de GPIO en I2C pour lire les boutons
// Motor : classe qui gère les moteurs pas à pas
// Button : lecture simple d'un bouton
// Inter  : interrupteur pour choisir LOCAL ou REMOTE

Adafruit_MCP23X17 mcp;
Motor motorElevation(config::PIN_MOTOR_ELEV_STEP, config::PIN_MOTOR_ELEV_DIR, config::PIN_MOTOR_ELEV_EN);
Motor motorCirculaire(config::PIN_MOTOR_CIRC_STEP, config::PIN_MOTOR_CIRC_DIR, config::PIN_MOTOR_CIRC_EN);
Motor motorFire(config::PIN_MOTOR_FIRE_STEP, config::PIN_MOTOR_FIRE_DIR, config::PIN_MOTOR_FIRE_EN);

Laser laser(config::PIN_LASER);
bool lastButtonState = false;

void startCameraServer();
void ws_watchdog();

// ---------------------------------------------------------------------------
// SETUP
// Cette fonction est exécutée une seule fois au démarrage de l'ESP32.
// Elle sert à initialiser tout le système.
// ---------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  Wire.begin(config::PIN_SDA, config::PIN_SCL);
  mcp.begin_I2C(0x20);
  Serial.setDebugOutput(true);
  Serial.println();

  // Création d'un réseau WiFi autonome
  // Le PC ou le téléphone se connecte directement à l'ESP32
  WiFi.softAP(config::WIFI_SSID, config::WIFI_PASSWORD);

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

  motorElevation.init();
  motorCirculaire.init();
  motorFire.init();
  motorElevation.setHold(true);
  motorFire.setStepInterval(config::MOTOR_FIRE_STEP_INTERVAL);
  motorFire.setInvert(true);
  motorElevation.setStepInterval(config::MOTOR_ELEV_STEP_INTERVAL);
  laser.init();
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

  // Arrêt de la tourelle si la page web ne répond plus
  ws_watchdog();
}
