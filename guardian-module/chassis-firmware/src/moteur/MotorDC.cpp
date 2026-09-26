#include "MotorDC.h"

MotorDC::MotorDC(int in1, int in2, int ena, int pwm) : _IN1(in1), _IN2(in2), _ENA(ena), _PWM(pwm)
{
    pinMode(_IN1, OUTPUT);
    pinMode(_IN2, OUTPUT);
    ledcAttach(_ENA, _PWM, 8);
}
void MotorDC::forward(int speed)
{
    digitalWrite(_IN1, HIGH);
    digitalWrite(_IN2, LOW);
    setSpeed(speed);
}
void MotorDC::backward(int speed)
{
    digitalWrite(_IN2, HIGH);
    digitalWrite(_IN1, LOW);
    setSpeed(speed);
}
void MotorDC::stop()
{
    digitalWrite(_IN1, HIGH);
    digitalWrite(_IN2, HIGH);
    ledcWrite(_ENA, 255);
}
void MotorDC::setSpeed(int speed)
{
    ledcWrite(_ENA, speed);
}
