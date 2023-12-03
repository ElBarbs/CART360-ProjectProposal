#include <FastLED.h>

#define DP 3

#define NUM_LEDS 50

CRGB leds[NUM_LEDS]{0};

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DP, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(200);
  FastLED.clear();
  FastLED.show();
}

int data = 0;

void loop() {
  static int ipos = 1;
  static int idir = 1;
  
  ipos += idir;
  if (ipos == NUM_LEDS| ipos == 0) {
    idir *= -1;
  }
  
  leds[ipos] = CRGB(255,0, 0); // Set LED color to red

  for ( int i = 0 ; i<NUM_LEDS ; ++ i ) {
    leds[i] = leds[i].fadeToBlackBy(100);
  }
  
  FastLED.show();

  if (Serial.available() > 0) {
    data = Serial.read();
    delay(10);
  }
  
  delay (50);
  // Had issue with reading data.
  //delay(800 - (data * 100));
}