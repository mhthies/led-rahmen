#include <FastLED.h>

#include "settings.hpp"
#include "patterns.hpp"

extern const uint16_t NUM_LEDS;
extern Settings SETTINGS;
extern CRGB leds[];

SimplePatternList gPatterns = { colorful, rainbow_fade, sparkles };
extern constexpr size_t gPatternsLen = sizeof(gPatterns) / sizeof(gPatterns[0]);

void colorful() 
{
  CHSV hsv;
  hsv.hue = SETTINGS.colorful_speed * millis() / 1000;
  hsv.val = 255;
  hsv.sat = ((uint16_t)SETTINGS.colorful_sat) * 5 / 2;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = hsv;
    hsv.hue += 255 / NUM_LEDS;
  }
}

void rainbow_fade()
{
  // TODO
  return;
}

void sparkles() {
  // TODO 
  return;
}
