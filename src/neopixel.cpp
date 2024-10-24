#include "neopixel.h"
#include <FastLED.h>

// Globale Variablen in neopixel.cpp definieren
CRGB leds[NUM_LEDS];
unsigned long previousNeopixelMillis[NUM_LEDS];
bool NeopixelState[NUM_LEDS];
float frequencies[NUM_LEDS];
CRGB originalColors[NUM_LEDS];

// Funktionsdefinitionen
void controlLED(int ledNum, CRGB color, float frequency) {
    if (ledNum < 0 || ledNum >= NUM_LEDS) return;

    originalColors[ledNum] = color;
    frequencies[ledNum] = frequency;

    if (frequency == 0) {
        leds[ledNum] = color;
        FastLED.show();
    }
}

void updateLEDs() {
    unsigned long currentMillis = millis();

    for (int i = 0; i < NUM_LEDS; i++) {
        if (frequencies[i] > 0) {
            unsigned long interval = 1000 / (frequencies[i] * 2);

            if (currentMillis - previousNeopixelMillis[i] >= interval) {
                previousNeopixelMillis[i] = currentMillis;
                NeopixelState[i] = !NeopixelState[i];

                if (NeopixelState[i]) {
                    leds[i] = originalColors[i];
                } else {
                    leds[i] = CRGB::Black;
                }
            }
        }
    }
    FastLED.show();
}
