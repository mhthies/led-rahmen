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
  hsv.hue = SETTINGS.colorful_speed * millis() / 3000;
  hsv.val = 255;
  hsv.sat = SETTINGS.colorful_sat * (uint16_t)255 / 100;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = hsv;
    hsv.hue += 255 / NUM_LEDS;
  }
}


static inline uint8_t rainbow_fade_calcHue(int x, int y) {
  return (uint8_t)(
    (sin8(SETTINGS.rainbow_fade_direction * 255 / 360) * y + cos8(SETTINGS.rainbow_fade_direction * 255 / 360) * x - 256)
      * 10 / (NUM_LEDS * SETTINGS.rainbow_fade_size));
}

void rainbow_fade()
{
  CHSV hsv;
  hsv.val = 255;
  hsv.sat = SETTINGS.rainbow_fade_sat * (uint16_t)255 / 100;

  uint8_t baseHue = SETTINGS.rainbow_fade_speed * millis() / 3000;
  for (unsigned int i = 0; i < NUM_LEDS/2; i++) {
    hsv.hue = rainbow_fade_calcHue(0, i) + baseHue;
    leds[i] = hsv;
  }
  for (unsigned int i = NUM_LEDS/2; i < NUM_LEDS; i++) {
    hsv.hue = rainbow_fade_calcHue(11, NUM_LEDS-i-1) + baseHue;
    leds[i] = hsv;
  }
}


void sparkles() {
  fadeToBlackBy(leds, NUM_LEDS, SETTINGS.sparkles_fade);
  if (random8(100) < SETTINGS.sparkles_rate) {
    uint8_t baseHue = millis() / 500;
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(baseHue + random8(48), SETTINGS.sparkles_sat * (uint16_t)255 / 100, 255); // TODO 
  }
}
