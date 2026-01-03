#ifndef MOTOR_PAN_H
#define MOTOR_PAN_H

#include <Arduino.h>

class MotorPan
{
public:
    enum MotorState
    {
        STOP,
        LEFT,
        RIGHT
    };

private:
    int pin1;
    int pin2;
    MotorState state;

public:
    MotorPan(int gpio1, int gpio2);

    void left();
    void right();
    void stop();

    MotorState getState();
};

#endif