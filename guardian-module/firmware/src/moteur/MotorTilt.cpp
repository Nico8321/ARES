#include "MotorTilt.h"

MotorTilt::MotorTilt(int gpio1, int gpio2)
{
    pin1 = gpio1;
    pin2 = gpio2;

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);

    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);

    state = STOP;
}

void MotorTilt::up()
{
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    state = UP;
}

void MotorTilt::down()
{
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    state = DOWN;
}

void MotorTilt::stop()
{
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    state = STOP;
}

MotorTilt::MotorState MotorTilt::getState()
{
    return state;
}