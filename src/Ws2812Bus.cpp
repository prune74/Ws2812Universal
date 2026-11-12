#include "Ws2812Bus.h"
#include "esp_log.h"
#include <string.h>

// Timing WS2812 — 80 MHz / clk_div 2 => tick 25 ns :
//   bit 0 : high 300 ns (12 ticks) / low 700 ns (28 ticks)
//   bit 1 : high 700 ns (28 ticks) / low 300 ns (12 ticks)
//   reset : low >= 50 us
static const uint16_t WS_T0H = 12;
static const uint16_t WS_T0L = 28;
static const uint16_t WS_T1H = 28;
static const uint16_t WS_T1L = 12;
static const uint16_t WS_RESET_TICKS = 2400; // 60 us

static const char *TAG = "Ws2812Bus";

Ws2812Bus *Ws2812Bus::buses[8] = {nullptr};

Ws2812Bus::Ws2812Bus()
    : _valid(false), _cfg()
{
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

    // v1.1.0 : buffer externe (zéro copie) OU buffer interne RAW
    if (cfg.extRgb888) {
        _ext = cfg.extRgb888;
        _buffer = nullptr;
        memset(_ext, 0, (size_t)_cfg.numLeds * 3);
    } else {
        _buffer = (uint32_t *)calloc(_cfg.numLeds, sizeof(uint32_t));
        if (!_buffer) return false;
    }

    _itemCount = (size_t)_cfg.numLeds * 32 + 1;
    _items = (rmt_item32_t *)malloc(_itemCount * sizeof(rmt_item32_t));
    if (!_items) {
        if (_buffer) { free(_buffer); _buffer = nullptr; }
        return false;
    }

    rmt_config_t rmt = RMT_DEFAULT_CONFIG_TX(_cfg.pin, _cfg.rmtChannel);
    rmt.clk_div = 2;
    rmt.mem_block_num = 1;
    rmt.flags = 0;
    esp_err_t err = rmt_config(&rmt);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_config canal %d : %s",
                 _cfg.rmtChannel, esp_err_to_name(err));
        return false;
    }

    err = rmt_driver_install(_cfg.rmtChannel, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install canal %d : %s",
                 _cfg.rmtChannel, esp_err_to_name(err));
        return false;
    }

    _valid = true;
    return true;
}

Ws2812Bus &Ws2812Bus::bus(uint8_t index)
{
    if (index >= 8) index = 0;
    if (!buses[index]) buses[index] = new Ws2812Bus();
    return *buses[index];
}

uint8_t Ws2812Bus::count()
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (buses[i] && buses[i]->_valid) n++;
    }
    return n;
}

// ---------------------------------------------------------------------------
// Framebuffer RAW (la luminosité est appliquée à l'ENCODAGE, v1.1.0)
// ---------------------------------------------------------------------------

void Ws2812Bus::setPixel(uint16_t index, uint32_t rgb)
{
    if (!_valid || index >= _cfg.numLeds) return;

    const uint8_t r = (rgb >> 16) & 0xFF;
    const uint8_t g = (rgb >> 8) & 0xFF;
    const uint8_t b = rgb & 0xFF;

    if (_ext) {
        uint8_t *p = _ext + (size_t)index * 3;
        p[0] = r; p[1] = g; p[2] = b;
    } else {
        _buffer[index] = rgb & 0x00FFFFFF; // RAW RGB, sans scaling
    }
}

void Ws2812Bus::_readPixel(uint16_t index, uint8_t &r, uint8_t &g, uint8_t &b) const
{
    if (_ext) {
        const uint8_t *p = _ext + (size_t)index * 3;
        r = p[0]; g = p[1]; b = p[2];
    } else {
        const uint32_t rgb = _buffer[index];
        r = (rgb >> 16) & 0xFF;
        g = (rgb >> 8) & 0xFF;
        b = rgb & 0xFF;
    }
}

void Ws2812Bus::fill(uint32_t rgb)
{
    if (!_valid) return;
    for (uint16_t i = 0; i < _cfg.numLeds; i++) setPixel(i, rgb);
}

void Ws2812Bus::clear()
{
    if (!_valid) return;
    if (_ext) memset(_ext, 0, (size_t)_cfg.numLeds * 3);
    else memset(_buffer, 0, _cfg.numLeds * sizeof(uint32_t));
}

// ---------------------------------------------------------------------------
// Émission RMT non bloquante — luminosité appliquée ICI (v1.1.0)
// ---------------------------------------------------------------------------

bool Ws2812Bus::busy()
{
    if (!_valid) return false;
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
    if (busy()) return false;

    const uint8_t br = _cfg.brightness;

    size_t k = 0;
    for (uint16_t i = 0; i < _cfg.numLeds; i++) {

        uint8_t r, g, b;
        _readPixel(i, r, g, b);

        // Scaling luminosité à l'encodage → réglable à chaud
        r = (uint8_t)((uint16_t)r * br / 255);
        g = (uint8_t)((uint16_t)g * br / 255);
        b = (uint8_t)((uint16_t)b * br / 255);

        // Ordre natif WS2812 : GRB, MSB en tête
        const uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

        for (int bit = 23; bit >= 0; bit--) {
            const bool one = (grb >> bit) & 0x01;
            _items[k].level0    = 1;
            _items[k].duration0 = one ? WS_T1H : WS_T0H;
            _items[k].level1    = 0;
            _items[k].duration1 = one ? WS_T1L : WS_T0L;
            k++;
        }
    }

    _items[k].level0    = 0;
    _items[k].duration0 = WS_RESET_TICKS;
    _items[k].level1    = 0;
    _items[k].duration1 = 0;
    k++;

    esp_err_t err = rmt_write_items(_cfg.rmtChannel, _items, k, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_write_items : %s", esp_err_to_name(err));
        return false;
    }
    return true;
}