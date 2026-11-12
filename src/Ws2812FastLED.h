#pragma once

#include <stdint.h>
#include "WsCrgb.h"
#include "Ws2812Bus.h"
#include "Ws2812All.h"

/**
 * Ws2812FastLED — façade compatible "FastLED-like" (v1.1.0).
 *
 * Objectif : migrer une application FastLED en touchant presque rien :
 *   • le buffer de l'application est utilisé TEL QUEL (zéro copie)
 *   • les canaux RMT sont EXPLICITES (fournis à addLeds) — aucune
 *     allocation séquentielle cachée
 *
 * Exemple EXCC :
 *   WsCrgb stripH_feux[9];
 *   Ws2812FastLED::addLeds(PIN_WS2812_H, RMT_CHANNEL_0, stripH_feux, 9);
 *   ...
 *   Ws2812FastLED::setBrightness(128);
 *   Ws2812FastLED::clear(true);
 *   Ws2812FastLED::show();     // = Ws2812All::showAll()
 */
class Ws2812FastLED
{
public:
    // Enregistre un strip : buffer EXTERNE {R,G,B}×numLeds (zéro copie),
    // canal RMT EXPLICITE (cohabitation DCC). 8 strips max.
    // Retourne l'index du strip (0..7) ou -1 si plein/config invalide.
    static int addLeds(gpio_num_t pin, rmt_channel_t channel,
                       WsCrgb *buffer, uint16_t numLeds,
                       uint8_t brightness = 128);

    static void show();                    // tous les strips, non bloquant
    static void setBrightness(uint8_t b);  // appliqué au prochain show()
    static void clear(bool writeNow = false); // buffer des strips à zéro

    static uint8_t stripCount();
    static uint32_t droppedShows();        // diagnostic (Ws2812All)
};