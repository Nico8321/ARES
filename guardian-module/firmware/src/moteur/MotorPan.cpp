#include "MotorPan.h"

MotorPan::MotorPan(int stepPin, int dirPin, int enPin)
{
    pinStep = stepPin;
    pinDir = dirPin;
    pinEn = enPin;

    pinMode(pinStep, OUTPUT);
    pinMode(pinDir, OUTPUT);
    pinMode(pinEn, OUTPUT);

    digitalWrite(pinStep, LOW);
    digitalWrite(pinDir, LOW);
    digitalWrite(pinEn, HIGH); // disabled by default

    state = STOP;
}

void MotorPan::enable()
{
    digitalWrite(pinEn, LOW);
}

void MotorPan::disable()
{
    digitalWrite(pinEn, HIGH);
}

void MotorPan::left()
{
    enable();
    digitalWrite(pinDir, HIGH);
    state = LEFT;
}

void MotorPan::right()
{
    enable();
    digitalWrite(pinDir, LOW);
    state = RIGHT;
}

void MotorPan::stop()
{
    disable();
    state = STOP;
}

void MotorPan::stepOnce()
{
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(5);
    digitalWrite(pinStep, LOW);
    delayMicroseconds(5);
}

MotorPan::MotorState MotorPan::getState()
{
    return state;
}