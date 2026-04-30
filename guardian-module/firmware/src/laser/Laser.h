#ifndef LASER_H
#define LASER_H

#include <Arduino.h>

// Classe Laser
// Permet de piloter un module laser via un GPIO (ON/OFF)

class Laser
{
private:
    int pin;
    bool state;

public:
    Laser(int pin);
    void init(); // Configure le GPIO et met le laser à OFF
    void on();
    void off();
    void toggle();
    bool getState();
};

#endif