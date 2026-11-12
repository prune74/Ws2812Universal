#pragma once

#include <stdint.h>

/**
 * WsCrgb — couleur RGB 3 octets, BINAIRE-compatible avec le CRGB de
 * FastLED (mêmes champs r,g,b dans cet ordre). Les tableaux
 * `WsCrgb strip[N]` se lisent donc comme des `CRGB strip[N]`.
 */
struct WsCrgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    WsCrgb() : r(0), g(0), b(0) {}
    WsCrgb(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
    // Couleur HTML 0xRRGGBB
    WsCrgb(uint32_t htmlColor)
        : r((htmlColor >> 16) & 0xFF),
          g((htmlColor >> 8) & 0xFF),
          b(htmlColor & 0xFF) {}

    WsCrgb &operator=(uint32_t htmlColor)
    {
        r = (htmlColor >> 16) & 0xFF;
        g = (htmlColor >> 8) & 0xFF;
        b = htmlColor & 0xFF;
        return *this;
    }

    WsCrgb &operator+=(const WsCrgb &o)
    {
        uint16_t rr = (uint16_t)r + o.r; r = rr > 255 ? 255 : rr;
        uint16_t gg = (uint16_t)g + o.g; g = gg > 255 ? 255 : gg;
        uint16_t bb = (uint16_t)b + o.b; b = bb > 255 ? 255 : bb;
        return *this;
    }

    bool operator==(const WsCrgb &o) const
    { return r == o.r && g == o.g && b == o.b; }

    bool operator!=(const WsCrgb &o) const { return !(*this == o); }

    // Couleurs nommées (API statique, comme CRGB::Green etc.)
    static WsCrgb Black()   { return WsCrgb(0, 0, 0); }
    static WsCrgb Red()     { return WsCrgb(0xFF, 0, 0); }
    static WsCrgb Green()   { return WsCrgb(0, 0xFF, 0); }
    static WsCrgb Blue()    { return WsCrgb(0, 0, 0xFF); }
    static WsCrgb White()   { return WsCrgb(0xFF, 0xFF, 0xFF); }
    static WsCrgb Orange()  { return WsCrgb(0xFF, 0xA5, 0); }
    static WsCrgb Yellow()  { return WsCrgb(0xFF, 0xFF, 0); }
    static WsCrgb Purple()  { return WsCrgb(0x80, 0, 0x80); }
    static WsCrgb Cyan()    { return WsCrgb(0, 0xFF, 0xFF); }
    static WsCrgb Magenta() { return WsCrgb(0xFF, 0, 0xFF); }
};

/**
 * ws_sin8 — approximation sinusoïdale 0..255, port exact du sin8 de
 * FastLED (utilisé par les animations, ex. EXCC_StatusLed).
 */
uint8_t ws_sin8(uint8_t theta);