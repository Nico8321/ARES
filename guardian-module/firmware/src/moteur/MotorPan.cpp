#include "MotorPan.h"

MotorPan::MotorPan(int gpio1, int gpio2)
{
    pin1 = gpio1;
    pin2 = gpio2;

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);

    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);

    state = STOP;
}

void MotorPan::left()
{
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    state = LEFT;
}

void MotorPan::right()
{
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    state = RIGHT;
}

void MotorPan::stop()
{
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    state = STOP;
}

MotorPan::MotorState MotorPan::getState()
{
    return state;
}