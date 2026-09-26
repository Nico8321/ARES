#include "MecanumChassis.h"
#include <Arduino.h>

MecanumChassis::MecanumChassis(MotorDC fl, MotorDC fr, MotorDC rl, MotorDC rr)
    : _fl(fl), _fr(fr), _rl(rl), _rr(rr)
{
}

void MecanumChassis::forward(int speed)
{
    // Toutes les roues en avant
    _fl.forward(speed);
    _fr.forward(speed);
    _rl.forward(speed);
    _rr.forward(speed);
}

void MecanumChassis::backward(int speed)
{
    // Toutes les roues en arrière
    _fl.backward(speed);
    _fr.backward(speed);
    _rl.backward(speed);
    _rr.backward(speed);
}

void MecanumChassis::strafeLeft(int speed)
{
    // Mecanum translation gauche :
    // FL arrière / FR avant / RL avant / RR arrière
    _fl.backward(speed);
    _fr.forward(speed);
    _rl.forward(speed);
    _rr.backward(speed);
}

void MecanumChassis::strafeRight(int speed)
{
    // Mecanum translation droite :
    // FL avant / FR arrière / RL arrière / RR avant
    _fl.forward(speed);
    _fr.backward(speed);
    _rl.backward(speed);
    _rr.forward(speed);
}

void MecanumChassis::rotateLeft(int speed)
{
    // Rotation gauche sur place :
    // gauche arrière / droite avant
    _fl.backward(speed);
    _fr.forward(speed);
    _rl.backward(speed);
    _rr.forward(speed);
}

void MecanumChassis::rotateRight(int speed)
{
    // Rotation droite sur place :
    // gauche avant / droite arrière
    _fl.forward(speed);
    _fr.backward(speed);
    _rl.forward(speed);
    _rr.backward(speed);
}

void MecanumChassis::stop()
{
    _fl.stop();
    _fr.stop();
    _rl.stop();
    _rr.stop();
}
