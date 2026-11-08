#pragma once

#include <stdint.h>
#include <driver/gpio.h>
#include <driver/rmt.h>

/**
 * ============================================================================
 *  Ws2812BusConfig
 * ============================================================================
 *  Structure décrivant la configuration d'un bus WS2812 unique.
 *  Miroir de CanBusConfig (CanUniversal) : la configuration est
 *  fournie par l'application, jamais codée en dur dans la bibliothèque.
 *
 *  ⚠️ Cohabitation DCC : le canal RMT est explicite. Le DCC occupe
 *  ses propres canaux RMT ; l'application doit choisir ici un canal
 *  distinct (défaut : RMT_CHANNEL_7, le plus haut de l'ESP32).
 * ============================================================================
 */
struct Ws2812BusConfig {

    bool enabled;            // Active ou désactive le bus
    gpio_num_t pin;          // Broche de données des LED
    uint16_t numLeds;        // Nombre de LED, libre (3 convertisseur, 9 hub…)
    rmt_channel_t rmtChannel; // Canal RMT (cohabitation DCC)
    uint8_t brightness;      // Luminosité globale 0..255

    // Constructeur par défaut sécurisé (bus désactivé)
    Ws2812BusConfig()
        : enabled(false),
          pin(GPIO_NUM_NC),
          numLeds(0),
          rmtChannel(RMT_CHANNEL_MAX),
          brightness(255)
    {}

    // Constructeur complet — permet l'usage de { ... } dans la config
    // maître de l'application
    Ws2812BusConfig(
        bool enabled,
        gpio_num_t pin,
        uint16_t numLeds,
        rmt_channel_t rmtChannel = RMT_CHANNEL_7,
        uint8_t brightness = 255
    )
        : enabled(enabled),
          pin(pin),
          numLeds(numLeds),
          rmtChannel(rmtChannel),
          brightness(brightness)
    {}
};

/**
 * ============================================================================
 *  Ws2812ConfigProvider
 * ============================================================================
 *  Miroir de CanConfigProvider : chaque module applicatif fournit sa
 *  propre configuration matérielle.
 *
 *  👉 Aucun code matériel dans la bibliothèque : ni pins, ni nombre de
 *     LED, ni canal RMT — tout est externalisé dans les modules CFC.
 * ============================================================================
 */
class Ws2812ConfigProvider
{
public:
    virtual ~Ws2812ConfigProvider() {}

    // Nombre de bus WS2812 déclarés par l'application
    virtual uint8_t busCount() const = 0;

    // Configuration du bus d'index donné (0..busCount()-1)
    virtual const Ws2812BusConfig &busConfig(uint8_t index) const = 0;
};