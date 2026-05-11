#pragma once

#include <Arduino.h>
#include "../Config.h"
class Motor
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
    bool _cycleActive = false;
    bool _goingUp = true;
    bool _holdPosition = false;
    bool _invertDir = false;

    int _targetSteps = 0;
    int _currentSteps = 0;

    unsigned long _lastStepTime = 0;
    unsigned long _stepInterval = config::MOTOR_DEFAULT_STEP_INTERVAL; // microsecondes entre steps
    MotorState state;

public:
    Motor(int stepPin, int dirPin, int enPin);

    void init();
    void enable();
    void disable();

    void up();
    void down();
    void stop();
    void stepOnce();

    void startFireCycle(int steps);
    void update();
    void setHold(bool hold);
    void setStepInterval(unsigned long interval);
    void setInvert(bool invert);
    MotorState getState();
};
