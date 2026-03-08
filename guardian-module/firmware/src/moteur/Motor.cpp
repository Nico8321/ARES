// ============================================================================
// Classe Motor
// Gestion des moteurs pas à pas via driver (type DRV8825).
// - génération des impulsions STEP
// - gestion de la direction
// - gestion du enable
// - gestion d'un cycle automatique pour le moteur de tir
// ============================================================================

#include "Motor.h"
#include <Adafruit_MCP23X17.h>

extern Adafruit_MCP23X17 mcp;

// Constructeur : enregistre les broches utilisées par le moteur
Motor::Motor(int stepPin, int dirPin, int enPin)
{
    pinStep = stepPin;
    pinDir = dirPin;
    pinEn = enPin;

    state = STOP;
}

// Initialisation des broches du moteur
// STEP est direct sur l'ESP32
// DIR et ENABLE passent par le MCP23017
void Motor::init()
{
    pinMode(pinStep, OUTPUT);
    mcp.pinMode(pinDir, OUTPUT);
    mcp.pinMode(pinEn, OUTPUT);

    digitalWrite(pinStep, LOW);
    mcp.digitalWrite(pinDir, LOW);
    mcp.digitalWrite(pinEn, HIGH);
}

// Active le driver moteur
void Motor::enable()
{
    mcp.digitalWrite(pinEn, LOW);
}

// Désactive le driver moteur
void Motor::disable()
{
    mcp.digitalWrite(pinEn, HIGH);
}

// Définit la direction UP et démarre le mouvement
void Motor::up()
{
    enable();
    mcp.digitalWrite(pinDir, LOW);
    state = UP;
    _lastStepTime = micros();
}

// Définit la direction DOWN et démarre le mouvement
void Motor::down()
{
    enable();
    mcp.digitalWrite(pinDir, HIGH);
    state = DOWN;
    _lastStepTime = micros();
}

// Arrête le mouvement du moteur
// Si holdPosition est faux, on coupe aussi le driver
void Motor::stop()
{
    if (!_holdPosition)
        disable();

    state = STOP;
}

// Génère une impulsion STEP pour faire avancer le moteur d'un pas
void Motor::stepOnce()
{
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(2); // suffisant pour DRV8825
    digitalWrite(pinStep, LOW);
}

// Retourne l'état actuel du moteur (UP, DOWN ou STOP)
Motor::MotorState Motor::getState()
{
    return state;
}

// Lance un cycle de tir :
// le moteur monte sur un certain nombre de steps puis redescend
void Motor::startFireCycle(int steps)
{
    if (_cycleActive)
        return;

    _targetSteps = steps;
    _currentSteps = 0;
    _goingUp = true;
    _cycleActive = true;

    enable();
    up();
}

// Fonction appelée en permanence dans loop()
// Génère les steps en respectant le timing (_stepInterval)
// Gère aussi le cycle automatique du moteur de tir
void Motor::update()
{
    unsigned long now = micros();

    // Priorité au cycle de tir
    if (_cycleActive)
    {
        if (now - _lastStepTime < _stepInterval)
            return;

        _lastStepTime = now;
        stepOnce();

        if (_goingUp)
        {
            _currentSteps++;
            if (_currentSteps >= _targetSteps)
            {
                down();
                _goingUp = false;
            }
        }
        else
        {
            _currentSteps--;
            if (_currentSteps <= 0)
            {
                _cycleActive = false;
                disable();
            }
        }
        return;
    }

    // Mouvement normal
    if (state == UP || state == DOWN)
    {
        if (now - _lastStepTime >= _stepInterval)
        {
            _lastStepTime = now;
            stepOnce();
        }
    }
}

// Active ou non le maintien du moteur en position
void Motor::setHold(bool hold)
{
    _holdPosition = hold;
}

// Définit la vitesse du moteur (intervalle entre deux steps)
void Motor::setStepInterval(unsigned long interval)
{
    _stepInterval = interval;
}