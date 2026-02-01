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
    int pinStep;
    int pinDir;
    int pinEn;
    MotorState state;

public:
    MotorTilt(int stepPin, int dirPin, int enPin);

    void enable();
    void disable();

    void up();   // sets DIR for UP
    void down(); // sets DIR for DOWN
    void stop(); // disables motor

    void stepOnce(); // single STEP pulse (slow tests)

    MotorState getState();
};

#endif