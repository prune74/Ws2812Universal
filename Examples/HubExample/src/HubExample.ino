/**
 * ============================================================================
 *  HubExample — Ws2812Universal v1.1.0
 * ============================================================================
 *  Cas d'usage : DCC_HUB_DISTRIB-Booster (hub de district).
 *  Un strip de 9 LED d'état, piloté en NON BLOQUANT pendant que le
 *  hub tourne (distribution, surveillance des boosters, entrées TOR).
 *
 *  Démontre :
 *    • Ws2812ConfigProvider — pins / LED / canal RMT explicites
 *    • Ws2812Init::begin() — initialisation tolérante
 *    • Ws2812All::showAll() + setBrightnessAll() + droppedShows() (v1.1.0)
 *    • luminosité réglable À CHAUD (appliquée à l'encodage, pas au buffer)
 *
 *  Câblage : une LED WS2812 sur GPIO 26, alim 5V, masse commune.
 * ============================================================================
 */

#include <Arduino.h>
#include "Ws2812Universal.h"

// ---------------------------------------------------------------------------
// Brochage (hub) — le canal RMT est EXPLICITE : le DCC du hub vit sur ses
// propres canaux, aucune collision possible.
// ---------------------------------------------------------------------------
static const gpio_num_t PIN_HUB_STRIP  = GPIO_NUM_26;
static const uint16_t   HUB_NUM_LEDS   = 9;
static const rmt_channel_t HUB_CHANNEL = RMT_CHANNEL_7;

// ---------------------------------------------------------------------------
// Configuration matérielle (pattern CanUniversal : la lib ne sait rien)
// ---------------------------------------------------------------------------
class HubLedConfig : public Ws2812ConfigProvider
{
public:
    uint8_t busCount() const override { return 1; }

    const Ws2812BusConfig &busConfig(uint8_t index) const override
    {
        static Ws2812BusConfig cfg(
            true,           // enabled
            PIN_HUB_STRIP,  // pin
            HUB_NUM_LEDS,   // 9 LED
            HUB_CHANNEL,    // canal RMT explicite
            255);           // luminosité (modifiable à chaud ensuite)

        return cfg;
    }
};

static HubLedConfig hubLedConfig;

// Couleurs d'état du district (0x00RRGGBB)
static const uint32_t COULEUR_OFF       = 0x000000;
static const uint32_t COULEUR_OK       = 0x004400;  // vert discret
static const uint32_t COULEUR_ACTIF     = 0x00AA00;  // distribution active
static const uint32_t COULEUR_DEFAUT   = 0xFF0000;   // booster en défaut
static const uint32_t COULEUR_COUPE     = 0xFFA500;   // district coupé (injection)

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[HubExample] Ws2812Universal v1.1.0");

    // Initialisation tolérante : si le canal RMT est pris ou la config
    // invalide, le bus est marqué invalide — jamais de crash.
    Ws2812Init::begin(hubLedConfig);

    Ws2812Bus &strip = Ws2812Bus::bus(0);
    if (!strip.isValid())
    {
        Serial.println("[HubExample] ERREUR : bus WS2812 invalide (canal ? pin ?)");
        return; // on ne bloque pas : le hub continue sans ses LED
    }

    Serial.printf("[HubExample] bus OK — %u LED, canal %d\n",
                  strip.numLeds(), (int)strip.channel());

    // Voyage de reconnaissance : chaque LED s'allume une fois
    for (uint16_t i = 0; i < strip.numLeds(); i++)
    {
        strip.clear();
        strip.setPixel(i, COULEUR_OK);
        strip.show();
        strip.waitDone();
        delay(60);
    }
}

void loop()
{
    static uint32_t phase = 0;
    static uint8_t  brightness = 255;
    static uint32_t dernierChgtBrightness = 0;

    Ws2812Bus &strip = Ws2812Bus::bus(0);
    if (!strip.isValid())
    {
        delay(1000);
        return;
    }

    // --- Simulation d'état : chenillard vert + LED défaut qui clignote ---
    const uint16_t n = strip.numLeds();
    strip.clear();

    // LED 0..7 : activité de distribution (chenillard lent)
    const uint16_t indexChenillard = (phase / 8) % n;
    for (uint16_t i = 0; i < n; i++)
        strip.setPixel(i, (i == indexChenillard) ? COULEUR_ACTIF : COULEUR_OK);

    // LED 8 : défaut distant (clignote orange)
    if ((phase / 32) & 0x01)
        strip.setPixel(n - 1, COULEUR_DEFAUT);

    // Émission globale (v1.1.0) : showAll() traverse TOUS les bus
    // valides — ici un seul, mais l'API est déjà multi-bus.
    Ws2812All::showAll();

    // --- Luminosité À CHAUD toutes les 2 s : 255 -> 64 -> 255 ---
    if (millis() - dernierChgtBrightness >= 2000)
    {
        dernierChgtBrightness = millis();
        brightness = (brightness == 255) ? 64 : 255;
        Ws2812All::setBrightnessAll(brightness);

        Serial.printf("[HubExample] luminosite=%u dropped=%lu\n",
                      brightness,
                      (unsigned long)Ws2812All::droppedShows());
    }

    phase++;
    delay(25); // ~40 Hz : fluide, et le RMT n'attend jamais
}