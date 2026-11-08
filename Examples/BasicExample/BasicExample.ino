/**
 * Ws2812Universal — Exemple de base (structure CanUniversal).
 *
 * Le module fournit sa configuration matérielle via le pattern
 * Provider : la bibliothèque ne connaît ni les pins, ni les canaux.
 *
 * Convertisseur : 1 LED sur GPIO 18.
 * Hub : passer numLeds à 9.
 *
 * ⚠️ Cohabitation DCC : le canal RMT choisi ici (7) doit rester
 * distinct de ceux utilisés par le DCC.
 */

#include <Ws2812Universal.h>

// --- Configuration matérielle fournie par le module (pattern Provider) ---

class MyWs2812Config : public Ws2812ConfigProvider {

    // Bus 0 : 1 LED (convertisseur) sur GPIO 18, canal RMT 7
    const Ws2812BusConfig bus0 {
        /* enabled   */ true,
        /* pin       */ GPIO_NUM_18,
        /* numLeds   */ 1,
        /* rmtChan   */ RMT_CHANNEL_7,   // cohabitation DCC
        /* brightness*/ 255
    };

public:

    uint8_t busCount() const override { return 1; }

    const Ws2812BusConfig &busConfig(uint8_t index) const override {
        return bus0;
    }
};

static MyWs2812Config ws2812Config;

void setup() {
    Serial.begin(115200);

    // Initialisation tolérante : un bus absent ne fait jamais crasher
    Ws2812Init::begin(ws2812Config);

    if (!Ws2812Bus::bus(0).isValid()) {
        Serial.println("Bus WS2812 invalide (matériel absent ?), "
                       "l'application continue.");
    }
}

void loop() {

    Ws2812Bus &bus = Ws2812Bus::bus(0);

    // Rouge -> vert -> bleu -> éteint, émission NON BLOQUANTE
    // (le DCC peut tourner en parallèle, aucune interruption coupée)

    bus.fill(0x200000);  // rouge
    bus.show();          // rend la main immédiatement
    delay(1000);

    bus.fill(0x002000);  // vert
    bus.show();
    delay(1000);

    bus.fill(0x000020);  // bleu
    bus.show();
    delay(1000);

    bus.clear();
    bus.show();
    delay(1000);
}