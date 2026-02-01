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
    int pinStep;
    int pinDir;
    int pinEn;
    MotorState state;

public:
    MotorPan(int stepPin, int dirPin, int enPin);

    void enable();
    void disable();

    void left();  // sets DIR for LEFT
    void right(); // sets DIR for RIGHT
    void stop();  // disables motor

    void stepOnce(); // single STEP pulse (slow tests)

    MotorState getState();
};

#endif