#include "Laser.h"

Laser::Laser(int pin) : pin(pin) {};

void Laser::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
}
void Laser::on()
{
    digitalWrite(pin, HIGH);
    state = true;
}
void Laser::off()
{
    digitalWrite(pin, LOW);
    state = false;
}
void Laser::toggle()
{
    if (state)
        off();
    else
        on();
}

bool Laser::getState()
{
    return state;
}