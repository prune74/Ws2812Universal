#pragma once

#include "Ws2812Bus.h"

/**
 * Ws2812All — opérations globales sur tous les bus enregistrés (v1.1.0).
 * showAll() n'attend jamais : un bus occupé saute son tour et compte
 * comme "dropped" (diagnostic), jamais de blocage.
 */
class Ws2812All
{
public:
    static uint8_t busCount();            // bus valides (0..8)
    static bool showAll();                // true si ≥ 1 bus a émis
    static void setBrightnessAll(uint8_t b);

    static uint32_t droppedShows();       // shows ignorés (bus busy)
    static void resetStats();
};