#pragma once

#include <stdint.h>
#include "Ws2812Config.h"

/**
 * 🚌 Ws2812Bus — Abstraction multi-bus WS2812, émission RMT non bloquante.
 * Miroir de CanBus (CanUniversal) :
 *   • registre statique : Ws2812Bus::bus(index) renvoie TOUJOURS une
 *     référence valide (jamais NULL, jamais de crash),
 *   • un bus absent, désactivé ou mal câblé est simplement « invalide »,
 *   • configuration matérielle externalisée (Provider).
 *
 * Implémentation : driver RMT legacy (IDF 4.4 / arduino-esp32 2.0.x).
 * Pas de bit-banging, aucune interruption coupée : le matériel RMT et
 * le ring buffer du driver font tout le travail, y compris pendant
 * que le DCC tourne sur ses propres canaux.
 */
class Ws2812Bus
{
public:

    // --- Registre statique (comme CanBus) ---

    // Accès sécurisé à un bus par index : la référence est toujours
    // valide ; l'instance est invalide si le bus est absent/en erreur.
    static Ws2812Bus &bus(uint8_t index);

    // Bus initialisé avec succès ?
    bool isValid() const { return _valid; }

    // --- Écriture du framebuffer ---

    // Fixe la couleur d'une LED (0x00RRGGBB). Sans effet si bus
    // invalide ou index hors limites — jamais de crash.
    void setPixel(uint16_t index, uint32_t rgb);

    // Remplit tout le bus avec une couleur.
    void fill(uint32_t rgb);

    // Éteint tout le bus (framebuffer à zéro, sans émettre).
    void clear();

    // --- Émission ---

    // Lance l'émission du framebuffer. NON BLOQUANT : la trame est
    // copiée dans le ring buffer RMT et la main est rendue immédiatement.
    // Retourne false si bus invalide ou trame précédente encore en cours.
    bool show();

    // Une émission est-elle en cours ? (sonde, non bloquant)
    bool busy();

    // Attend la fin de l'émission en cours (bloquant, usage libre).
    void waitDone();

    // --- Accesseurs ---

    uint16_t numLeds() const { return _cfg.numLeds; }
    rmt_channel_t channel() const { return _cfg.rmtChannel; }

private:

    // Seul Ws2812Init crée les instances (pattern CanBus)
    friend class Ws2812Init;

    Ws2812Bus();                          // instance invalide
    bool begin(const Ws2812BusConfig &cfg);

    bool _valid = false;
    Ws2812BusConfig _cfg;

    // Framebuffer GRB (ordre natif WS2812), luminosité déjà appliquée
    uint32_t *_buffer = nullptr;

    // Symboles RMT pré-alloués (32 bits/LED + reset)
    rmt_item32_t *_items = nullptr;
    size_t _itemCount = 0;

    static Ws2812Bus *buses[8];
};