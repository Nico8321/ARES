#pragma once

namespace config
{
    constexpr int PIN_SDA = 13;
    constexpr int PIN_SCL = 14;
    constexpr int PIN_MOTOR_ELEV_STEP = 32;
    constexpr int PIN_MOTOR_ELEV_DIR = 8;
    constexpr int PIN_MOTOR_ELEV_EN = 9;
    constexpr int PIN_MOTOR_CIRC_STEP = 33;
    constexpr int PIN_MOTOR_CIRC_DIR = 10;
    constexpr int PIN_MOTOR_CIRC_EN = 11;
    constexpr int PIN_MOTOR_FIRE_STEP = 12;
    constexpr int PIN_MOTOR_FIRE_DIR = 12;
    constexpr int PIN_MOTOR_FIRE_EN = 13;

    constexpr int PIN_LASER = 23;
    constexpr const char *WIFI_SSID = "ARES";
    constexpr const char *WIFI_PASSWORD = "12345678";
    constexpr unsigned long MOTOR_ELEV_STEP_INTERVAL = 11000;
    constexpr unsigned long MOTOR_FIRE_STEP_INTERVAL = 2000;
    constexpr int FIRE_CYCLE = 1200;
    constexpr unsigned long MOTOR_DEFAULT_STEP_INTERVAL = 6000;
    constexpr int HTTP_PORT = 80;
}
