#include <pebble.h>
#include "weather.h"

WeatherInfo Weather_weatherInfo;
WeatherForecastInfo Weather_weatherForecast;

GDrawCommandImage* Weather_currentWeatherIcon;
GDrawCommandImage* Weather_forecastWeatherIcon;

uint32_t getConditionIcon(WeatherCondition conditionCode) {
  uint32_t iconToLoad;

  switch(conditionCode) {
    case CLEAR_DAY:
      iconToLoad = RESOURCE_ID_WEATHER_CLEAR_DAY;
      break;
    case CLEAR_NIGHT:
      iconToLoad = RESOURCE_ID_WEATHER_CLEAR_NIGHT;
      break;
    case CLOUDY_DAY:
      iconToLoad = RESOURCE_ID_WEATHER_CLOUDY;
      break;
    case HEAVY_RAIN:
      iconToLoad = RESOURCE_ID_WEATHER_HEAVY_RAIN;
      break;
    case HEAVY_SNOW:
      iconToLoad = RESOURCE_ID_WEATHER_HEAVY_SNOW;
      break;
    case LIGHT_RAIN:
      iconToLoad = RESOURCE_ID_WEATHER_LIGHT_RAIN;
      break;
    case LIGHT_SNOW:
      iconToLoad = RESOURCE_ID_WEATHER_LIGHT_SNOW;
      break;
    case PARTLY_CLOUDY_NIGHT:
      iconToLoad = RESOURCE_ID_WEATHER_PARTLY_CLOUDY_NIGHT;
      break;
    case PARTLY_CLOUDY:
      iconToLoad = RESOURCE_ID_WEATHER_PARTLY_CLOUDY;
      break;
    case RAINING_AND_SNOWING:
      iconToLoad = RESOURCE_ID_WEATHER_RAINING_AND_SNOWING;
      break;
    case THUNDERSTORM:
      iconToLoad = RESOURCE_ID_WEATHER_THUNDERSTORM;
      break;
    default:
      iconToLoad = RESOURCE_ID_WEATHER_GENERIC;
      break;
  }

  return iconToLoad;
}

void Weather_setCurrentCondition(int conditionCode) {

  uint32_t currentWeatherIcon = getConditionIcon(conditionCode);

  // ok, now load the new icon:
  gdraw_command_image_destroy(Weather_currentWeatherIcon);
  Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(currentWeatherIcon);

  Weather_weatherInfo.currentIconResourceID = currentWeatherIcon;
}

// void Weather_setForecastCondition(int conditionCode) {
//   uint32_t forecastWeatherIcon = getConditionIcon(forecastCondition);
//
//   gdraw_command_image_destroy(Weather_forecastWeatherIcon);
//   Weather_forecastWeatherIcon = gdraw_command_image_create_with_resource(forecastWeatherIcon);
//
//   Weather_weatherForecast.forecastIconResourceID = forecastWeatherIcon;
// }

void Weather_init() {
  // if possible, load weather data from persistent storage
  printf("starting weather!");
  if (persist_exists(WEATHERINFO_PERSIST_KEY)) {
    printf("current key exists!");
    WeatherInfo w;
    persist_read_data(WEATHERINFO_PERSIST_KEY, &w, sizeof(WeatherInfo));

    Weather_weatherInfo = w;

    Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(w.currentIconResourceID);

  } else {

    printf("current key does not exist!");
    // otherwise, use null data
    Weather_currentWeatherIcon = NULL;
    Weather_weatherInfo.currentTemp = INT32_MIN;
  }

  if (persist_exists(WEATHERFORECAST_PERSIST_KEY)) {
    printf("forecast key exists!");
    WeatherForecastInfo w;
    persist_read_data(WEATHERFORECAST_PERSIST_KEY, &w, sizeof(WeatherForecastInfo));

    Weather_weatherForecast = w;

    Weather_forecastWeatherIcon = gdraw_command_image_create_with_resource(w.forecastIconResourceID);

  } else {
    printf("forecast key does not exist!");

    Weather_forecastWeatherIcon = NULL;
    Weather_weatherForecast.highTemp = INT32_MIN;
    Weather_weatherForecast.lowTemp = INT32_MIN;
  }
}

void Weather_saveData() {
  printf("saving data!");
  persist_write_data(WEATHERINFO_PERSIST_KEY, &Weather_weatherInfo, sizeof(WeatherInfo));
  persist_write_data(WEATHERFORECAST_PERSIST_KEY, &Weather_weatherForecast, sizeof(WeatherForecastInfo));
}

void Weather_deinit() {
  // save weather data to persistent storage
  Weather_saveData();

  // free memory
  gdraw_command_image_destroy(Weather_currentWeatherIcon);
  gdraw_command_image_destroy(Weather_forecastWeatherIcon);
}
