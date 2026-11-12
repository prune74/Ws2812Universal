#include "WsCrgb.h"

uint8_t ws_sin8(uint8_t theta)
{
    static const uint8_t b_m16_interleave[] = {
        0, 49, 49, 41, 90, 27, 117, 10,
        4, 244, 122, 248, 4, 205, 208, 2
    };

    uint8_t offset = theta;
    if (theta & 0x40) offset = (uint8_t)255 - offset;
    offset &= 0x3F;

    uint8_t secoffset = offset & 0x0F;
    if (theta & 0x40) ++secoffset;

    uint8_t m  = b_m16_interleave[secoffset];
    uint8_t b  = offset >> 4;
    uint8_t x  = (uint8_t)(((uint16_t)b * b * m) + b);
    x >>= 2;
    if (theta & 0x40) x = 255 - x;
    return x;
}