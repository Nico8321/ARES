#ifndef MOTOR_TILT_H
#define MOTOR_TILT_H

#include <Arduino.h>

class MotorTilt
{
public:
    enum MotorState
    {
        STOP,
        UP,
        DOWN
    };

private:
    int pin1;
    int pin2;
    MotorState state;

public:
    MotorTilt(int gpio1, int gpio2);

    void up();
    void down();
    void stop();

    MotorState getState();
};

#endif