#pragma once
#include <pebble.h>

// persistent storage
#define WEATHERINFO_PERSIST_KEY 2
#define WEATHERFORECAST_PERSIST_KEY 222

typedef struct {
  int currentTemp;
  uint32_t currentIconResourceID;
} WeatherInfo;

typedef struct {
  int highTemp;
  int lowTemp;
  uint32_t forecastIconResourceID;
} WeatherForecastInfo;

extern WeatherInfo Weather_weatherInfo;
extern WeatherForecastInfo Weather_weatherForecast;

extern GDrawCommandImage* Weather_currentWeatherIcon;
extern GDrawCommandImage* Weather_forecastWeatherIcon;


void Weather_setConditions(int conditionCode, bool isNight, int forecastCondition);
void Weather_saveData();
void Weather_init();
void Weather_deinit();
