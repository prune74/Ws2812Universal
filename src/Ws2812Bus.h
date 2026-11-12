#pragma once

#include <stdint.h>
#include "Ws2812Config.h"

/**
 * Ws2812Bus — abstraction multi-bus WS2812, émission RMT non bloquante.
 * v1.1.0 :
 *   • framebuffer RAW (luminosité appliquée à l'ENCODAGE → réglable à chaud)
 *   • setBrightness() — visible au prochain show(), sans réécriture
 *   • buffer externe RGB888 possible (extRgb888 de la config)
 */
class Ws2812Bus
{
public:

    static Ws2812Bus &bus(uint8_t index);

    bool isValid() const { return _valid; }

    // --- Framebuffer (couleurs RAW, sans luminosité) ---

    void setPixel(uint16_t index, uint32_t rgb);  // 0x00RRGGBB
    void fill(uint32_t rgb);
    void clear();

    // --- Émission ---

    bool show();       // non bloquant ; false si busy/invalid
    bool busy();
    void waitDone();

    // --- v1.1.0 : luminosité à chaud (appliquée au prochain show()) ---
    void setBrightness(uint8_t b) { _cfg.brightness = b; }
    uint8_t brightness() const { return _cfg.brightness; }

    // --- Accesseurs ---
    uint16_t numLeds() const { return _cfg.numLeds; }
    rmt_channel_t channel() const { return _cfg.rmtChannel; }

    // Registre (miroir CanBus) — accès protégé pour Ws2812All
    static uint8_t count();   // nb de bus valides (parmi 8)

private:

    friend class Ws2812Init;
    friend class Ws2812FastLED;   // v1.1.0 : la façade crée des bus

    Ws2812Bus();
    bool begin(const Ws2812BusConfig &cfg);

    // Lecture RAW d'une LED (r,g,g) selon le mode interne/externe
    void _readPixel(uint16_t index, uint8_t &r, uint8_t &g, uint8_t &b) const;

    bool _valid = false;
    Ws2812BusConfig _cfg;

    uint32_t *_buffer = nullptr;   // interne : RAW RGB (0x00RRGGBB)
    uint8_t *_ext = nullptr;       // externe : {R,G,B}×numLeds (non possédé)

    rmt_item32_t *_items = nullptr;
    size_t _itemCount = 0;

    static Ws2812Bus *buses[8];
};