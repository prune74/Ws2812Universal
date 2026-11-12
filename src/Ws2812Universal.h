#pragma once

/**
 * 💡 Ws2812Universal v1.1.0 — WS2812 pour ESP32 (architecture CFC)
 *   • Émission RMT non bloquante, canal EXPLICITE par l'application
 *   • Multi-bus : Ws2812Bus::bus(index), jamais NULL, jamais de crash
 *   • v1.1.0 : Ws2812All (showAll/setBrightnessAll/droppedShows),
 *     WsCrgb (compatible CRGB), Ws2812FastLED (façade FastLED-like,
 *     buffers externes zéro copie), luminosité à chaud
 */
#include "Ws2812Config.h"
#include "Ws2812Bus.h"
#include "Ws2812Init.h"
#include "Ws2812All.h"
#include "WsCrgb.h"
#include "Ws2812FastLED.h"