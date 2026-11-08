#include "Ws2812Bus.h"
#include "esp_log.h"
#include <string.h>

// Timing WS2812 — base RMT 80 MHz / clk_div 2 => tick 25 ns :
//   bit 0 : high 300 ns (12 ticks) / low 700 ns (28 ticks)
//   bit 1 : high 700 ns (28 ticks) / low 300 ns (12 ticks)
//   reset : low >= 50 us
static const uint16_t WS_T0H = 12;
static const uint16_t WS_T0L = 28;
static const uint16_t WS_T1H = 28;
static const uint16_t WS_T1L = 12;
static const uint16_t WS_RESET_TICKS = 2400; // 60 us

static const char *TAG = "Ws2812Bus";

// Registre statique des bus (pattern CanBus)
Ws2812Bus *Ws2812Bus::buses[8] = {nullptr};

// ---------------------------------------------------------------------------
// 🟦 Construction / initialisation
// ---------------------------------------------------------------------------

Ws2812Bus::Ws2812Bus()
    : _valid(false), _cfg()
{
    // Instance invalide par défaut (bus absent / en erreur)
}

bool Ws2812Bus::begin(const Ws2812BusConfig &cfg)
{
    if (_valid) return true;

    if (cfg.pin == GPIO_NUM_NC || cfg.numLeds == 0 ||
        cfg.rmtChannel >= RMT_CHANNEL_MAX) {
        ESP_LOGE(TAG, "configuration invalide (pin, numLeds ou canal RMT)");
        return false;
    }

    _cfg = cfg;

    // Framebuffer GRB
    _buffer = (uint32_t *)calloc(_cfg.numLeds, sizeof(uint32_t));
    if (!_buffer) return false;

    // Symboles RMT : 32 bits par LED + 1 symbole de reset
    _itemCount = (size_t)_cfg.numLeds * 32 + 1;
    _items = (rmt_item32_t *)malloc(_itemCount * sizeof(rmt_item32_t));
    if (!_items) {
        free(_buffer);
        _buffer = nullptr;
        return false;
    }

    // Canal TX RMT (driver legacy, arduino-esp32 2.0.x).
    // Le canal est celui choisi par l'application : aucun conflit
    // avec les canaux RMT déjà utilisés par le DCC.
    rmt_config_t rmt = RMT_DEFAULT_CONFIG_TX(_cfg.pin, _cfg.rmtChannel);
    rmt.clk_div = 2;          // tick de 25 ns
    rmt.mem_block_num = 1;
    rmt.flags = 0;
    esp_err_t err = rmt_config(&rmt);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_config canal %d : %s",
                 _cfg.rmtChannel, esp_err_to_name(err));
        return false;
    }

    // Le driver recharge le périphérique par interruption via son ring
    // buffer : rmt_write_items(..., false) ne bloque jamais l'appelant
    // et ne coupe aucune interruption (DCC inclus).
    err = rmt_driver_install(_cfg.rmtChannel, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install canal %d : %s",
                 _cfg.rmtChannel, esp_err_to_name(err));
        return false;
    }

    memset(_buffer, 0, _cfg.numLeds * sizeof(uint32_t));
    _valid = true;
    return true;
}

// ---------------------------------------------------------------------------
// 🟪 Accès statique sécurisé (comme CanBus::bus)
// ---------------------------------------------------------------------------

Ws2812Bus &Ws2812Bus::bus(uint8_t index)
{
    if (index >= 8) index = 0;
    if (!buses[index]) buses[index] = new Ws2812Bus(); // instance invalide
    return *buses[index];
}

// ---------------------------------------------------------------------------
// 🟨 Framebuffer
// ---------------------------------------------------------------------------

void Ws2812Bus::setPixel(uint16_t index, uint32_t rgb)
{
    if (!_valid || index >= _cfg.numLeds) return;

    const uint8_t r = (rgb >> 16) & 0xFF;
    const uint8_t g = (rgb >> 8)  & 0xFF;
    const uint8_t b = rgb & 0xFF;

    // Luminosité globale appliquée à l'écriture
    const uint8_t rb = (uint8_t)((uint16_t)r * _cfg.brightness / 255);
    const uint8_t gb = (uint8_t)((uint16_t)g * _cfg.brightness / 255);
    const uint8_t bb = (uint8_t)((uint16_t)b * _cfg.brightness / 255);

    // Ordre natif WS2812 : GRB, MSB en tête
    _buffer[index] = ((uint32_t)gb << 16) | ((uint32_t)rb << 8) | bb;
}

void Ws2812Bus::fill(uint32_t rgb)
{
    if (!_valid) return;
    for (uint16_t i = 0; i < _cfg.numLeds; i++) setPixel(i, rgb);
}

void Ws2812Bus::clear()
{
    if (!_valid) return;
    memset(_buffer, 0, _cfg.numLeds * sizeof(uint32_t));
}

// ---------------------------------------------------------------------------
// 🟥 Émission RMT non bloquante
// ---------------------------------------------------------------------------

bool Ws2812Bus::busy()
{
    if (!_valid) return false;
    // Timeout 0 => simple sonde, ne bloque jamais
    return rmt_wait_tx_done(_cfg.rmtChannel, 0) != ESP_OK;
}

void Ws2812Bus::waitDone()
{
    if (!_valid) return;
    rmt_wait_tx_done(_cfg.rmtChannel, portMAX_DELAY);
}

bool Ws2812Bus::show()
{
    if (!_valid) return false;

    // Ne pas écraser une trame encore en cours de transmission
    if (busy()) return false;

    // Encodage des 24 bits GRB de chaque LED en symboles RMT.
    // Pour 1..9 LED la trame tient dans le ring buffer en un appel ;
    // le driver enchaîne par interruption sinon.
    size_t k = 0;
    for (uint16_t i = 0; i < _cfg.numLeds; i++) {
        const uint32_t grb = _buffer[i];
        for (int bit = 23; bit >= 0; bit--) {
            const bool one = (grb >> bit) & 0x01;
            _items[k].level0    = 1;
            _items[k].duration0 = one ? WS_T1H : WS_T0H;
            _items[k].level1    = 0;
            _items[k].duration1 = one ? WS_T1L : WS_T0L;
            k++;
        }
    }

    // Symbole de reset (latch), niveau bas >= 50 us
    _items[k].level0    = 0;
    _items[k].duration0 = WS_RESET_TICKS;
    _items[k].level1    = 0;
    _items[k].duration1 = 0;
    k++;

    // NON BLOQUANT : copie dans le ring buffer du driver, l'émission
    // matérielle se fait toute seule, sans couper la moindre interruption.
    esp_err_t err = rmt_write_items(_cfg.rmtChannel, _items, k, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_write_items : %s", esp_err_to_name(err));
        return false;
    }
    return true;
}