#pragma once

#include <Arduino.h>
class Inter
{
private:
    int pin;

public:
    enum Mode
    {
        LOCAL,
        REMOTE
    };
    Inter(int gpio);

    void init();
    Mode getMode();
};
