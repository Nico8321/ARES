#pragma once
#include "MotorDC.h"

/**
 * @brief Contrôle un chassis Mecanum 4 roues via 2x TB6612
 *
 * Disposition des roues :
 *   FL ------ FR
 *   |          |
 *   RL ------ RR
 *
 * Roues Mecanum : FL et RR = rôles identiques / FR et RL = rôles identiques
 */
class MecanumChassis
{
private:
    MotorDC _fl; ///< Front Left
    MotorDC _fr; ///< Front Right
    MotorDC _rl; ///< Rear Left
    MotorDC _rr; ///< Rear Right

public:
    /**
     * @brief Constructeur
     * @param fl Moteur avant-gauche
     * @param fr Moteur avant-droit
     * @param rl Moteur arrière-gauche
     * @param rr Moteur arrière-droit
     * @param stby1 Pin STBY TB6612 #1
     * @param stby2 Pin STBY TB6612 #2
     */
    MecanumChassis(MotorDC fl, MotorDC fr, MotorDC rl, MotorDC rr);

    void forward(int speed = 200);     ///< Avance tout droit
    void backward(int speed = 200);    ///< Recule tout droit
    void strafeLeft(int speed = 200);  ///< Translation latérale gauche
    void strafeRight(int speed = 200); ///< Translation latérale droite
    void rotateLeft(int speed = 200);  ///< Rotation sur place gauche
    void rotateRight(int speed = 200); ///< Rotation sur place droite
    void stop();                       ///< Frein actif tous moteurs
};
