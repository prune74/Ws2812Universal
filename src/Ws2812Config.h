#pragma once

#include <stdint.h>
#include <driver/gpio.h>
#include <driver/rmt.h>

/**
 * Ws2812BusConfig — configuration d'un bus WS2812 (miroir de CanBusConfig).
 * v1.1.0 : champ extRgb888 (buffer externe de l'application, zéro copie).
 */
struct Ws2812BusConfig {

    bool enabled;
    gpio_num_t pin;
    uint16_t numLeds;
    rmt_channel_t rmtChannel;
    uint8_t brightness;      // 0..255 — appliquée à l'encodage (v1.1.0)

    // v1.1.0 : buffer externe {R,G,B}×numLeds possédé par l'application
    // (style FastLED). nullptr = buffer interne alloué par la lib.
    uint8_t *extRgb888;

    Ws2812BusConfig()
        : enabled(false),
          pin(GPIO_NUM_NC),
          numLeds(0),
          rmtChannel(RMT_CHANNEL_MAX),
          brightness(255),
          extRgb888(nullptr)
    {}

    Ws2812BusConfig(
        bool enabled,
        gpio_num_t pin,
        uint16_t numLeds,
        rmt_channel_t rmtChannel = RMT_CHANNEL_7,
        uint8_t brightness = 255,
        uint8_t *extRgb888 = nullptr
    )
        : enabled(enabled),
          pin(pin),
          numLeds(numLeds),
          rmtChannel(rmtChannel),
          brightness(brightness),
          extRgb888(extRgb888)
    {}
};

/**
 * Ws2812ConfigProvider — la configuration matérielle est fournie par
 * l'application (pins, LED, canaux RMT) : rien de codé en dur ici.
 */
class Ws2812ConfigProvider
{
public:
    virtual ~Ws2812ConfigProvider() {}

    virtual uint8_t busCount() const = 0;
    virtual const Ws2812BusConfig &busConfig(uint8_t index) const = 0;
};