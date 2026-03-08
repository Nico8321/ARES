#include "Button.h"
// On inclut le header de la classe Button
#include <Adafruit_MCP23X17.h>

extern Adafruit_MCP23X17 mcp;
Button::Button(int gpio)
{
    pin = gpio;    // On mémorise le numéro de la broche du bouton
    state = false; // Par défaut, le bouton est considéré comme "non appuyé"

    // On configure la broche en entrée avec résistance de pull-up interne
    // Ça veut dire :
    // - au repos -> la pin est à HIGH
    // - bouton appuyé -> la pin est reliée à la masse -> LOW
}
void Button::init()
{
    mcp.pinMode(pin, INPUT_PULLUP);
}

bool Button::isPressed()
{
    // On lit l'état électrique réel de la broche
    // digitalRead(pin) renvoie HIGH ou LOW
    bool reading = (mcp.digitalRead(pin) == LOW);
    // Ici, on transforme ça en logique :
    // LOW  -> bouton appuyé -> true
    // HIGH -> bouton relâché -> false

    state = reading; // On mémorise l'état du bouton
    return state;    // On renvoie true ou false au programme principal
}

bool Button::getState()
{
    return state; // Permet de récupérer le dernier état mémorisé
}