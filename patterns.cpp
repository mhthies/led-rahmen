#include <FastLED.h>

#include "settings.hpp"
#include "patterns.hpp"
#include "constants.h"

extern const uint16_t NUM_LEDS;
extern Settings SETTINGS;
extern CRGB leds[];

SimplePatternList gPatterns = { colorful, rainbow_fade, sparkles, fire };
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


#define COOLING  60
#define SPARKING 30

void fire() {
  // Code from https://github.com/FastLED/FastLED/blob/962b1205a7824ff79e2e585f221759accb2ecfd9/examples/Fire2012WithPalette/Fire2012WithPalette.ino

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS/2];
  static byte driftframe = 0;

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS/2; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS/2) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    driftframe++;
    if (driftframe >= 2) {
      for( int k= NUM_LEDS/2 - 1; k >= 2; k--) {
        heat[k] = (heat[k] + heat[k - 1] + heat[k - 2]/2 + heat[k - 2]/2 ) / 3;
      }
      driftframe = 0;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(4);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS/2; j++) {
      // Scale the heat value from 0-255 down to 0-220
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 220);
      CRGB color = ColorFromPalette( HeatColors_p, colorindex);
      leds[j] = color;
      leds[NUM_LEDS - j-1] = color;
    }
}
