#ifndef INTER_H
#define INTER_H
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
#endif
