#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "moteur/MecanumChassis.h"
#include "serveur/Serveur.h"
MecanumChassis chassis(
    MotorDC(PIN_FL_IN1, PIN_FL_IN2, PIN_FL_PWM, MOTOR_PWM_FREQ),
    MotorDC(PIN_FR_IN1, PIN_FR_IN2, PIN_FR_PWM, MOTOR_PWM_FREQ),
    MotorDC(PIN_RL_IN1, PIN_RL_IN2, PIN_RL_PWM, MOTOR_PWM_FREQ),
    MotorDC(PIN_RR_IN1, PIN_RR_IN2, PIN_RR_PWM, MOTOR_PWM_FREQ));

/*void testMove(const char *label, int duration_ms)
{
  Serial.print(">>> ");
  Serial.println(label);
  delay(duration_ms);
  chassis.stop();
  Serial.println("    STOP");
  delay(1000);
}
*/
void setup()
{

  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  // mDNS
  MDNS.begin(MDNS_NAME);

  // Démarrer le serveur HTTP
  startChassisServer();

  Serial.println("ARES Chassis prêt");
}

void loop()
{
}