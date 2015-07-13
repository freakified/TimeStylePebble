#pragma once
#include <pebble.h>

// persistent storage
#define WEATHERINFO_PERSIST_KEY 2
  
typedef struct {
  int currentTemp;
  uint32_t currentIconResourceID;
} WeatherInfo;
  
WeatherInfo Weather_weatherInfo;
GDrawCommandImage *Weather_currentWeatherIcon;

void Weather_setCondition(int conditionCode, bool isNight);
void Weather_init();
void Weather_deinit();