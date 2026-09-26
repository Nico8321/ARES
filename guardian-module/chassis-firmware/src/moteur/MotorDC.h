#pragma once
#include <Arduino.h>

/**
 * @brief Pilote un moteur DC via un pont en H (L298N ou compatible)
 */
class MotorDC
{
private:
    int _IN1; ///< Pin direction 1
    int _IN2; ///< Pin direction 2
    int _ENA; ///< Pin PWM vitesse
    int _PWM; ///< Fréquence PWM en Hz

public:
    /**
     * @brief Constructeur
     * @param in1 Pin IN1 du pont en H
     * @param in2 Pin IN2 du pont en H
     * @param ena Pin ENA pour le PWM
     * @param pwm Fréquence PWM en Hz (défaut 1000)
     */
    MotorDC(int in1, int in2, int ena, int pwm = 1000);

    /**
     * @brief Fait tourner le moteur en sens horaire
     * @param speed Vitesse PWM (0-255)
     */
    void forward(int speed = 255);

    /**
     * @brief Fait tourner le moteur en sens antihoraire
     * @param speed Vitesse PWM (0-255)
     */
    void backward(int speed = 255);

    /**
     * @brief Arrête le moteur et coupe le PWM
     */
    void stop();

    /**
     * @brief Définit la vitesse sans changer le sens
     * @param speed Vitesse PWM (0-255)
     */
    void setSpeed(int speed);
};