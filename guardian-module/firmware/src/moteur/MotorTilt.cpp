#include "MotorTilt.h"

MotorTilt::MotorTilt(int stepPin, int dirPin, int enPin)
{
    pinStep = stepPin;
    pinDir = dirPin;
    pinEn = enPin;

    pinMode(pinStep, OUTPUT);
    pinMode(pinDir, OUTPUT);
    pinMode(pinEn, OUTPUT);

    digitalWrite(pinStep, LOW);
    digitalWrite(pinDir, LOW);
    digitalWrite(pinEn, HIGH); // EN is active LOW -> disabled by default

    state = STOP;
}

void MotorTilt::enable()
{
    digitalWrite(pinEn, LOW); // enable driver
}

void MotorTilt::disable()
{
    digitalWrite(pinEn, HIGH); // disable driver
}

void MotorTilt::up()
{
    enable();
    digitalWrite(pinDir, HIGH);
    state = UP;
}

void MotorTilt::down()
{
    enable();
    digitalWrite(pinDir, LOW);
    state = DOWN;
}

void MotorTilt::stop()
{
    disable();
    state = STOP;
}

void MotorTilt::stepOnce()
{
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(5);
    digitalWrite(pinStep, LOW);
    delayMicroseconds(5);
}

MotorTilt::MotorState MotorTilt::getState()
{
    return state;
}