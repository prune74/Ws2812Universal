#include "Ws2812FastLED.h"
#include <string.h>

static WsCrgb *s_stripBuf[8] = {nullptr};
static uint16_t s_stripN[8] = {0};
static uint8_t s_stripCount = 0;

int Ws2812FastLED::addLeds(gpio_num_t pin, rmt_channel_t channel,
                           WsCrgb *buffer, uint16_t numLeds,
                           uint8_t brightness)
{
    if (s_stripCount >= 8) return -1;
    if (!buffer || numLeds == 0 || channel >= RMT_CHANNEL_MAX ||
        pin == GPIO_NUM_NC)
        return -1;

    // Le strip occupe l'index du registre = index du bus Ws2812Bus
    const uint8_t index = s_stripCount;

    Ws2812BusConfig cfg(true, pin, numLeds, channel, brightness,
                        (uint8_t *)buffer);   // buffer externe, zéro copie

    if (!Ws2812Bus::bus(index).begin(cfg))
        return -1;

    s_stripBuf[index] = buffer;
    s_stripN[index] = numLeds;
    s_stripCount++;
    return index;
}

void Ws2812FastLED::show()
{
    Ws2812All::showAll();
}

void Ws2812FastLED::setBrightness(uint8_t b)
{
    Ws2812All::setBrightnessAll(b);
}

void Ws2812FastLED::clear(bool writeNow)
{
    for (uint8_t i = 0; i < s_stripCount; i++)
        memset(s_stripBuf[i], 0, (size_t)s_stripN[i] * 3);
    if (writeNow) show();
}

uint8_t Ws2812FastLED::stripCount() { return s_stripCount; }

uint32_t Ws2812FastLED::droppedShows() { return Ws2812All::droppedShows(); }