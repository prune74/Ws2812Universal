/**
 * ============================================================================
 *  FastLEDCompatExample — Ws2812Universal v1.1.0
 * ============================================================================
 *  Test de la FAÇADE Ws2812FastLED : le chemin que prendra
 *  Extension_CantonController pour quitter FastLED.
 *
 *  Démontre :
 *    • Ws2812FastLED::addLeds(pin, canal, buffer, n) — buffers EXTERNES
 *      de l'application, zéro copie, canaux RMT EXPLICITES
 *    • WsCrgb — binaire-compatible CRGB (couleurs nommées via API statique)
 *    • setBrightness() / clear(true) / show() — l'API "FastLED-like"
 *    • ws_sin8() — port exact du sin8 FastLED (animations)
 *
 *  Câblage : 2 strips WS2812 — un sur GPIO 18 (9 LED), un sur GPIO 19
 *  (4 LED). Canaux RMT 0 et 1 : comme sur EXCC (strips 0..6, DCC = 7).
 * ============================================================================
 */

#include <Arduino.h>
#include "Ws2812Universal.h"

// ---------------------------------------------------------------------------
// Buffers de l'APPLICATION — comme sous FastLED, la lib ne les possède pas
// ---------------------------------------------------------------------------
static WsCrgb stripFeux[9]; // "mât signal"  : 9 LED
static WsCrgb stripDir[4];  // "direction"   : 4 LED

static const gpio_num_t PIN_FEUX = GPIO_NUM_18;
static const gpio_num_t PIN_DIR = GPIO_NUM_19;

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[FastLEDCompat] Ws2812Universal v1.1.0 — facade");

    // Enregistrement des strips : pins + canaux EXPLICITES, buffers externes.
    // Aucune allocation cachée, aucun canal choisi à notre insu.
    int s1 = Ws2812FastLED::addLeds(PIN_FEUX, RMT_CHANNEL_0, stripFeux, 9);
    int s2 = Ws2812FastLED::addLeds(PIN_DIR, RMT_CHANNEL_1, stripDir, 4);

    Serial.printf("[FastLEDCompat] strips enregistres : %d, %d (attendu 0, 1)\n",
                  s1, s2);

    // Comme FastLED : efface tout et affiche immédiatement
    Ws2812FastLED::setBrightness(128);
    Ws2812FastLED::clear(true);
}

void loop()
{
    static uint32_t t = 0;
    const uint8_t theta = (t * 4) & 0xFF; // phase d'animation

    // --- Mât : "feux" qui respirent en vert (ws_sin8, comme EXCC_StatusLed) ---
    const uint8_t respiration = 40 + (ws_sin8(theta) >> 2);
    for (uint8_t i = 0; i < 9; i++)
        stripFeux[i] = WsCrgb(0, respiration, 0);

    // Œilleton rouge fixe (index 8)
    stripFeux[8] = WsCrgb::Red();

    // --- Direction : barre cumulative orange qui monte et descend ---
    const uint8_t niveau = ws_sin8(theta);           // 0..255
    const uint8_t seuil = map(niveau, 0, 255, 0, 4); // 0..4 LED allumées
    for (uint8_t i = 0; i < 4; i++)
        stripDir[i] = (i < seuil) ? WsCrgb::Orange() : WsCrgb::Black();

    // Émission globale — show() traverse tous les strips enregistrés
    Ws2812FastLED::show();

    // Luminosité À CHAUD toutes les 3 s (visible sans toucher aux couleurs)
    static uint32_t dernierChgt = 0;
    static bool plein = true;
    if (millis() - dernierChgt >= 3000)
    {
        dernierChgt = millis();
        plein = !plein;
        Ws2812FastLED::setBrightness(plein ? 200 : 60);
        Serial.printf("[FastLEDCompat] brightness=%u dropped=%lu\n",
                      plein ? 200 : 60,
                      (unsigned long)Ws2812FastLED::droppedShows());
    }

    t++;
    delay(10);
}