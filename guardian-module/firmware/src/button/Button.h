#pragma once
#include <Arduino.h>
class Button
{
private:
    int pin;    // numero de la pin
    bool state; // booleen pour l'etat

public:
    Button(int gpio);

    void init();
    bool isPressed(); // methode pour si appuyer
    bool getState();  // methode pour recuperer l'etat
};
