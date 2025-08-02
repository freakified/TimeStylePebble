#pragma once
#include <pebble.h>

// persistent storage
#define WEATHER_PERSIST_KEY 223

typedef struct {
  // current weather
  int currentTemp;
  uint32_t currentIconResourceID;
  int currentUVIndex;

  // today's forecast
  int todaysHighTemp;
  int todaysLowTemp;
  uint32_t forecastIconResourceID;
} WeatherInfo;

typedef enum {
  CLEAR_DAY           = 0,
  CLEAR_NIGHT         = 1,
  CLOUDY_DAY          = 2,
  HEAVY_RAIN          = 3,
  HEAVY_SNOW          = 4,
  LIGHT_RAIN          = 5,
  LIGHT_SNOW          = 6,
  PARTLY_CLOUDY_NIGHT = 7,
  PARTLY_CLOUDY       = 8,
  RAINING_AND_SNOWING = 9,
  THUNDERSTORM        = 10,
  WEATHER_GENERIC     = 11
} WeatherCondition;

extern WeatherInfo Weather_weatherInfo;

extern GDrawCommandImage* Weather_currentWeatherIcon;
extern GDrawCommandImage* Weather_forecastWeatherIcon;

void Weather_setCurrentCondition(int conditionCode);
void Weather_setForecastCondition(int conditionCode);
void Weather_saveData();
void Weather_init();
void Weather_deinit();
