#pragma once
#include <pebble.h>

// persistent storage
#define WEATHERINFO_PERSIST_KEY 2

typedef struct {
  int currentTemp;
  uint32_t currentIconResourceID;
} WeatherInfo;

static WeatherInfo Weather_weatherInfo;

#ifdef PBL_COLOR
  extern GDrawCommandImage* Weather_currentWeatherIcon;
#else
  extern GBitmap* Weather_currentWeatherIcon;
#endif


void Weather_setCondition(int conditionCode, bool isNight);
void Weather_init();
void Weather_deinit();
