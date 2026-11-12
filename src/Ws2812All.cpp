#include "Ws2812All.h"

static uint32_t s_dropped = 0;

uint8_t Ws2812All::busCount() { return Ws2812Bus::count(); }

bool Ws2812All::showAll()
{
    bool any = false;
    for (uint8_t i = 0; i < 8; i++) {
        Ws2812Bus &b = Ws2812Bus::bus(i);
        if (!b.isValid()) continue;
        if (b.show()) any = true;
        else s_dropped++;   // busy ou erreur → reporté au prochain cycle
    }
    return any;
}

void Ws2812All::setBrightnessAll(uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++) {
        Ws2812Bus &bus = Ws2812Bus::bus(i);
        if (bus.isValid()) bus.setBrightness(b);
    }
}

uint32_t Ws2812All::droppedShows() { return s_dropped; }
void Ws2812All::resetStats() { s_dropped = 0; }