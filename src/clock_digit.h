#pragma once
  
#include <pebble.h>

  
/*
 * Represents a single digit, as shown on the clock. 
 */
typedef struct {
  int currentNum;
  GColor bgColor;
  GColor fgColor;
  uint32_t currentImageId;
  GBitmap* currentImage;
  BitmapLayer* imageLayer;
} ClockDigit;

/*
 * Sets the number shown. Allocates the appropriate background image.
 */
void ClockDigit_setNumber(ClockDigit* this, int number);

void ClockDigit_setColor(ClockDigit* this, GColor fg, GColor bg);

void ClockDigit_construct(ClockDigit* this, GPoint pos);
void ClockDigit_destruct(ClockDigit* this);