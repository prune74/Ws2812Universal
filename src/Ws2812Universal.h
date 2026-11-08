#pragma once

/**
 * 💡 Ws2812Universal — Bibliothèque WS2812 pour ESP32 (architecture CFC)
 * Miroir de CanUniversal.h : header parapluie, inclure ce seul fichier.
 *
 *   • Émission RMT non bloquante (pas de bit-banging,
 *     aucune interruption coupée — le DCC tourne en parallèle)
 *   • Canal RMT configurable par l'application (cohabitation DCC)
 *   • Multi-bus : Ws2812Bus::bus(index), jamais NULL, jamais de crash
 *   • Configuration matérielle externalisée (Provider), comme CanUniversal
 *   • Cible : arduino-esp32 2.0.x (espressif32@6.9.0)
 *   • Usage : 1 LED (convertisseur) ou 9 LED (hub)
 *
 * Exemple :
 *
 *   class MyConfig : public Ws2812ConfigProvider { ... };
 *   MyConfig config;
 *   Ws2812Init::begin(config);        // initialisation tolérante
 *   Ws2812Bus &bus = Ws2812Bus::bus(0);
 *   bus.fill(0x000020);
 *   bus.show();                       // non bloquant
 */

#include "Ws2812Config.h"
#include "Ws2812Bus.h"
#include "Ws2812Init.h"