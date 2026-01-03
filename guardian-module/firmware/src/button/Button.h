#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>
class Button
{
private:
    int pin;    // numero de la pin
    bool state; // booleen pour l'etat

public:
    Button(int gpio);
    bool isPressed(); // methode pour si appuyer
    bool getState();  // methode pour recuperer l'etat
};
#endif