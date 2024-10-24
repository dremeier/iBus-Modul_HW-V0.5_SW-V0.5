/* Neopixel Status LEDs*/

#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <FastLED.h>

// Neopixel-Konfiguration
#define NUM_LEDS 3         // Anzahl LEDs
#define LED_PIN 3
#define BRIGHTNESS 40
#define LED_TYPE NEOPIXEL

// Externe Deklaration der globalen Variablen
extern CRGB leds[];
extern unsigned long previousNeopixelMillis[];
extern bool NeopixelState[];
extern float frequencies[];
extern CRGB originalColors[];

// Funktionsdeklarationen
void controlLED(int ledNum, CRGB color, float frequency);
void updateLEDs();

#endif
