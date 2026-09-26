#pragma once

// ============================================================================
// MOTEURS - TB6612 #1 (FL + FR)
// ============================================================================
constexpr int PIN_FL_IN1 = 4;
constexpr int PIN_FL_IN2 = 5;
constexpr int PIN_FL_PWM = 6;

constexpr int PIN_FR_IN1 = 7;
constexpr int PIN_FR_IN2 = 8;
constexpr int PIN_FR_PWM = 9;

// ============================================================================
// MOTEURS - TB6612 #2 (RL + RR)
// ============================================================================
constexpr int PIN_RL_IN1 = 10;
constexpr int PIN_RL_IN2 = 11;
constexpr int PIN_RL_PWM = 12;

constexpr int PIN_RR_IN1 = 13;
constexpr int PIN_RR_IN2 = 14;
constexpr int PIN_RR_PWM = 15;

// ============================================================================
// MOTEURS - PARAMETRES
// ============================================================================
constexpr int MOTOR_PWM_FREQ = 10000; // 10kHz, adapté JGB37-520
constexpr int MOTOR_MAX_SPEED = 255;
constexpr int MOTOR_MIN_SPEED = 0;

// ============================================================================
// WIFI / SERVEUR HTTP
// ============================================================================
constexpr const char *SSID = "ARES";
constexpr const char *PASSWORD = "12345678";
constexpr const char *MDNS_NAME = "AresChassis";
constexpr int HTTP_PORT = 80;