/**
 * Functions to calculate the sunrise and sunset times for a given location.
 * 
 * From: https://github.com/ewedel/pebble-sunclock/
 */

#pragma once


//  Per http://www.timeanddate.com/worldclock/aboutastronomy.html
//  each of the twilight bands is 6 degrees wide:
//     twilight type  horizon rel.   zenith rel.
//     astronomical   -12   .. -18   102   .. 108
//     nautical        -6   .. -12    96   .. 102
//     civil           -0.83 .. -6    90.83 .. 96
//     rise/set           -0.83          90.83

#define ZENITH_OFFICIAL 90.83333   /* sun rise/set angle: 90 degrees 50' */
#define ZENITH_CIVIL    96.0       /* darkest part of civil twilight */
#define ZENITH_NAUTICAL 102.0      /* darkest part of nautical twilight */
#define ZENITH_ASTRONOMICAL 108.0  /* darkest part of astronomical twilight */


/**
 *  Value returned by calcSunRise / calcSunSet when there is no rise / set for the
 *  specified parameters.  This likely means too close to the pole on the given date.
 */
#define NO_RISE_SET_TIME  ((float) 100.0)  /* (legal values are hours in a day) */

float calcSunRise(int year, int month, int day, float latitude, float longitude, float zenith);
float calcSunSet(int year, int month, int day, float latitude, float longitude, float zenith);

/**
 *  Adjust UTC hour + fraction to same in local time.
 */
float adjustTimezone(float time, float tzOffset);