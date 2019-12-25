// Christmas Tree Star
// Copyright 2019 Robert Beckman

#define ARM_MATH_CM4
#include <arm_math.h>
#include <Adafruit_NeoPixel.h>

// Output pin for power LED (pin 13 to use Teensy 3.0's onboard LED).
const int POWER_LED_PIN = 13;    
// Output pin for neo pixels.      
const int NEO_PIXEL_PIN = 2;  
// Number of neo pixels.         
const int NEO_PIXEL_COUNT = 40;
//Speed. Larger is slower
const int SPEED = 4;

//Current Brightness
int ost;
//If we're getting brighter or darker
int dir;
//Number of cycles we're running
int num;
//The current base position
int pos;
bool movePos;
//Whether to invert half the pixel positions
bool invertPos;
//Hues
int randHue1;
int randHue2;
int nextRandHue1;
int nextRandHue2;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_PIXEL_COUNT, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
////////////////////////////////////////////////////////////////////////////////
// MAIN SKETCH FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Set up serial port.
  Serial.begin(9600);

  ost = 0;
  dir = 1;
  num = 5;
  pos = 0;
  movePos = true;
  invertPos = false;
  randHue1 = random(6);
  randHue2 = random(6);
  nextRandHue1 = random(6);
  nextRandHue2 = random(6);
  
  // Turn on the power indicator LED.
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);
  
  // Initialize neo pixel library and turn off the LEDs
  pixels.begin();
  pixels.show(); 
}

void loop() {
   brighten();
}

void brighten() {
  int hues[] = {120, 235, 265, 60, 30, 0};

  //Start brightening if we're getting too dark.
  if(ost <= 10 && dir < 0) {
    dir = 1;
    //If we're out of cycles, set up for next cycle.
    if(num == 0) {
      randHue1 = nextRandHue1;
      randHue2 = nextRandHue2;
      nextRandHue1 = random(6);
      nextRandHue2 = random(6);
      num = 1 + random(4);
      movePos = (bool) random(2);
      if(!movePos) {
        pos = 0;
      }
      invertPos = (bool) random(2);
    }
  }

  //Start darkening if we're getting too bright
  if(ost >= 118 && dir > 0) {
    dir = -1;
    num--;
  }
  
  int hue1 = hues[randHue1];
  int hue2 = hues[randHue2];
  uint32_t color1;
  uint32_t color2;
  
  color1 = pixelHSVtoRGBColor(hue1, 1, ((float)ost)/256);
  color2 = pixelHSVtoRGBColor(hue2, 1, ((float)ost)/256);

  if(ost <= 128 && dir < 0 && num == 0) {
    int hue1b = hueBlend(hue1, hues[nextRandHue1], ((float)ost - 10) / 120);
    int hue2b = hueBlend(hue2, hues[nextRandHue2], ((float)ost - 10) / 120);
    color1 = pixelHSVtoRGBColor(hue1b, 1, ((float)ost)/256);
    color2 = pixelHSVtoRGBColor(hue2b, 1, ((float)ost)/256);
  }

  //Color our pixels
  colorAllPosition(color1, (0 + pos/SPEED) % 4);
  colorAllPosition(colorBlend(color1, color2, .33), (1 + pos/SPEED) % 4);
  colorAllPosition(colorBlend(color1, color2, .66), (2 + pos/SPEED) % 4);
  colorAllPosition(color2, (3 + pos/SPEED) % 4);

  //Increment the position
  if(movePos) {
    pos++;
    if(pos >= 4*SPEED) {
      pos = 0;
    }
  }

  ost = ost + 2*dir;
  pixels.show();
  delay(35);
}

////////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/**
 * Colors all pixels in a particular position on a star point. 
 * Position 0 is closest to the center, position 3 is a point.
 */
void colorAllPosition(uint32_t color, int pos) {
  for(int i = 0; i < 5; i++) {
    pixels.setPixelColor(8*i + pos, color);
    if(invertPos)
      pixels.setPixelColor(8*i + pos + 4, color);
    else
      pixels.setPixelColor(8*(i+1) - (pos + 1), color);
  }
}

int hueBlend(int hue1, int hue2, float s) {
  return (int)(hue1*s + hue2*(1-s));
}

/**
 *  Blends two colors by a percentage between 0-1. 
 *  s = 0.0 is 100% c1, s = 1.0 is 100% c2
 */
uint32_t colorBlend(uint32_t c1, uint32_t c2, float s) {
  int r1 = c1 >> 16;
  int g1 = (c1 >> 8) & 0xFF;
  int b1 = c1 & 0xFF;
  
  int r2 = c2 >> 16;
  int g2 = (c2 >> 8) & 0xFF;
  int b2 = c2 & 0xFF;
  
  int r = r2*s + r1*(1.0-s);
  int g = g2*s + g1*(1.0-s);
  int b = b2*s + b1*(1.0-s);

  return pixels.Color(r, g, b);
}

/** 
 *  Convert from HSV values (in floating point 0 to 1.0) to RGB colors usable
 *  by neo pixel functions.
 */ 
uint32_t pixelHSVtoRGBColor(float hue, float saturation, float value) {
  // Implemented from algorithm at http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
  float chroma = value * saturation;
  float h1 = float(hue)/60.0;
  float x = chroma*(1.0-fabs(fmod(h1, 2.0)-1.0));
  float r = 0;
  float g = 0;
  float b = 0;
  if (h1 < 1.0) {
    r = chroma;
    g = x;
  }
  else if (h1 < 2.0) {
    r = x;
    g = chroma;
  }
  else if (h1 < 3.0) {
    g = chroma;
    b = x;
  }
  else if (h1 < 4.0) {
    g = x;
    b = chroma;
  }
  else if (h1 < 5.0) {
    r = x;
    b = chroma;
  }
  else // h1 <= 6.0
  {
    r = chroma;
    b = x;
  }
  float m = value - chroma;
  r += m;
  g += m;
  b += m;
  
  return pixels.Color(int(255*r), int(255*g), int(255*b));
}
