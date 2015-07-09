#pragma once

/*
 * Responsible for background picture selection.
 * Based on sunrise and sunset times, splits the day into several "day parts",
 * each of which is associated with a background image.
 */
  
#include "daypart.h"
#include "location_info.h"

#define NUMBER_OF_DAYPARTS 8


// default day times
#define NOON_MINUTE 720
#define DAY_END_MINUTE 1440
#define TWILIGHT_DURATION 30
#define SUNRISE_DURATION 60

// default sunrise at 6:00am, default sunset at 6:00pm:
#define DEFAULT_SUNRISE_TIME 360
#define DEFAULT_SUNSET_TIME 1080
  
// persistent storage
#define BGPICKER_LOC_KEY 1

static Daypart bgpicker_dayparts[NUMBER_OF_DAYPARTS];
static GColor bgpicker_currentBackgroundColor;
static LocationInfo bgpicker_location;

/*
 * Sets up the set of 7 dayparts with their respective background images,
 * and times.
 */
void bgpicker_init();

/*
 * Returns a GColor for the current background color,
 * based on the specified time.
 */
GColor bgpicker_getCurrentBg(const struct tm* time);

/*
 * Updates the location, causing the sunrise and sunset values
 * to recalculate.
 */
void bgpicker_updateLocation(LocationInfo loc);

/*
 * Sets the sunrise and sunset times, causing all dayparts' start/end
 * times to be updated
 */
static void bgpicker_setSunriseSunset(int sunriseMinute, int sunsetMinute);