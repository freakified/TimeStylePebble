/**
 *  Math functions required by sun_calc. Taken from
 *  https://github.com/orgemd/pebble-sunclock
 */

#pragma once

//  NB: IEEE single-precision float has a 24 bit significand,
//      or about 7.2 significant decimal digits.
//      Presumably, the extras given here don't hurt anything.
#define M_PI 3.141592653589793

float my_sqrt(const float x);
float my_floor(float x); 
float my_fabs(float x);
float my_atan(float x);
float my_rint (float x);
float my_sin (float x);
float my_cos(float x);
float my_acos (float x);
float my_asin (float x);
float my_tan(float x);
float my_max(float x, float y);