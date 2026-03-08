
# ARES
**ARES – Assisted Remote Engagement System**

ARES est une plateforme robotique expérimentale basée sur **ESP32-CAM** combinant vision embarquée, contrôle moteur et interface web.

Le système permet de contrôler à distance l’orientation d’une tourelle robotisée et de visualiser en temps réel le flux vidéo de la caméra via une interface web embarquée.

Le projet combine plusieurs domaines :

- vision embarquée
- contrôle de moteurs pas à pas
- interface web embarquée
- contrôle local et distant
- robotique expérimentale

---

# Description du système

ARES est une plateforme robotique expérimentale permettant d’explorer différents concepts de robotique embarquée.

Le prototype actuel utilise un **mécanisme airsoft basse puissance** intégré à la tourelle afin de tester le contrôle mécanique du système et la synchronisation entre :

- les moteurs
- l’interface web
- le retour vidéo
- les commandes locales

L’objectif principal du projet est d’expérimenter des systèmes embarqués combinant **vision, contrôle moteur et interface réseau**.

---

# Fonctionnalités

- Streaming vidéo temps réel depuis l’ESP32-CAM
- Contrôle des moteurs de la tourelle via interface web
- Mode WiFi autonome (ESP32 en point d’accès)
- Accès via `http://ares.local`
- Contrôle local via boutons physiques
- Sélecteur de mode **LOCAL / REMOTE**
- Mécanisme de tir contrôlé par moteur pas à pas
- Architecture firmware non bloquante

---

# Matériel

- ESP32-CAM (WROVER)
- 3 moteurs pas à pas **NEMA17**
- Drivers **DRV8825**
- Extension GPIO **MCP23017**
- Boutons physiques de contrôle
- Alimentation **12V**
- Mécanisme airsoft basse puissance (expérimental)

---

# Architecture du firmware

Le firmware est structuré autour de plusieurs modules :

## Motor
Gestion générique des moteurs pas à pas :

- génération des impulsions STEP
- gestion des directions
- gestion des intervalles de pas
- fonctionnement non bloquant via `update()`

## Button
Lecture des boutons physiques via l’extension GPIO.

## Inter
Gestion du sélecteur de mode :

- LOCAL
- REMOTE

## Camera
Gestion du streaming vidéo via le serveur HTTP intégré.

## main.cpp
Orchestration générale du système :

- initialisation matériel
- configuration WiFi
- gestion des moteurs
- lecture des commandes

---

# Modes de fonctionnement

## Mode REMOTE

Le contrôle est effectué via l’interface web :

- orientation de la tourelle
- déclenchement du mécanisme
- visualisation du flux vidéo

## Mode LOCAL

Le contrôle est effectué via les boutons physiques présents sur le robot.

---

# Connexion

L’ESP32 crée son propre réseau WiFi :

SSID : ARES
Mot de passe : 12345678

Interface web :

http://ares.local

ou

http://192.168.4.1

---

# Compilation

Le firmware est développé avec **PlatformIO**.

Pour compiler le projet :

pio run

Pour flasher l’ESP32 :

pio run -t upload

Pour ouvrir le moniteur série :

pio device monitor

---

# Statut

Projet expérimental en développement.

Le firmware actuel est **fonctionnel sur le matériel** :

- contrôle des moteurs
- streaming vidéo
- contrôle local via boutons
- contrôle distant via interface web
- mécanisme airsoft fonctionnel

Cette architecture pose les bases pour les évolutions futures du système **ARES**.

---

# Note de sécurité

ARES est un projet expérimental destiné à l’apprentissage et à la recherche personnelle en robotique embarquée.

Le système doit être utilisé uniquement dans un environnement privé et contrôlé.

L'utilisateur est responsable de respecter les réglementations locales concernant l'utilisation de dispositifs airsoft ou similaires.
