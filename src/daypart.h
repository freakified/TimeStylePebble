#pragma once
  
#include <pebble.h>

/*
 * Represents a part of the day. The bounds are determined
 * by startMinute and endMinute, which are in the form of
 * minutes in a day (so, for example, 12:00 noon is 60*12 = 720)
 */
typedef struct {
  int startMinute;
  int endMinute;
  GColor bgColor;
} Daypart;

/*
 * Check if a given time is contained within a specified daypart
 */
bool daypart_containsTime(const Daypart* this, int minute);
