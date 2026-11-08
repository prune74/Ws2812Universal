#include "Ws2812Init.h"
#include "Ws2812Config.h"
#include "Ws2812Bus.h"
#include "esp_log.h"

/**
 * ============================================================================
 * 🚀 Ws2812Init — Initialisation multi-bus WS2812
 * ============================================================================
 * Comportement (miroir de CanInit) :
 *   - Chaque bus est initialisé indépendamment
 *   - Un bus en échec (pin invalide, canal RMT occupé…) est marqué
 *     invalide mais reste accessible via Ws2812Bus::bus(index)
 *   - Un bus désactivé (enabled=false) est attaché invalide
 *   - Aucun pointeur NULL n'est jamais exposé à l'application
 *
 * Résultat :
 *   → setup() propre et lisible
 *   → architecture multi-bus préservée
 *   → aucun Guru Meditation même si le matériel est absent
 * ============================================================================
 */

static const char *TAG = "Ws2812Init";

bool Ws2812Init::begin(const Ws2812ConfigProvider &provider)
{
    const uint8_t count = provider.busCount();

    for (uint8_t i = 0; i < count && i < 8; i++) {

        const Ws2812BusConfig &cfg = provider.busConfig(i);

        // Bus désactivé par l'application → invalide, sans erreur
        if (!cfg.enabled) {
            Ws2812Bus::bus(i); // instance invalide attachée au registre
            ESP_LOGW(TAG, "bus %u désactivé, attaché invalide", i);
            continue;
        }

        // Création et initialisation du bus
        Ws2812Bus &bus = Ws2812Bus::bus(i);
        if (bus.begin(cfg)) {
            ESP_LOGI(TAG, "bus %u OK : %u LED sur GPIO %d, canal RMT %d",
                     i, cfg.numLeds, cfg.pin, cfg.rmtChannel);
        } else {
            // Erreur → bus reste invalide mais accessible, pas de crash
            ESP_LOGE(TAG, "bus %u : initialisation échouée, bus invalide", i);
        }
    }

    // Initialisation tolérante : toujours true
    return true;
}