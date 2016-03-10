#include <pebble.h>
#include "weather.h"

WeatherInfo Weather_weatherInfo;
WeatherForecastInfo Weather_weatherForecast;

GDrawCommandImage* Weather_currentWeatherIcon;
GDrawCommandImage* Weather_forecastWeatherIcon;

uint32_t getConditionIcon(int conditionCode) {
  uint32_t iconToLoad;

  switch(conditionCode) {
    case 0: // tornado
    case 1: // tropical storm
    case 2: // hurricane
    case 3: // severe thunderstorms
    case 4: // thunderstorms
    case 37: //isolated thunderstorms
    case 38: //scattered thunderstorms
    case 39: //scattered thunderstorms
    case 45: //thundershowers
    case 47: //isolated thundershowers
      iconToLoad = RESOURCE_ID_WEATHER_THUNDERSTORM;
      break;
    case 5: //mixed rain and snow
    case 6: //mixed rain and sleet
    case 7: //mixed snow and sleet
    case 8: //freezing drizzle
    case 10: //freezing rain
    case 35: //mixed rain and hail
      iconToLoad = RESOURCE_ID_WEATHER_RAINING_AND_SNOWING;
      break;
    case 9: //drizzle
    case 40: //scattered showers
      iconToLoad = RESOURCE_ID_WEATHER_LIGHT_RAIN;
      break;
    case 11: //showers
    case 12: //showers
      iconToLoad = RESOURCE_ID_WEATHER_HEAVY_RAIN;
      break;
    case 13: //snow flurries
    case 14: //light snow showers
    case 15: //blowing snow
    case 17: //hail
    case 42: //scattered snow showers
      iconToLoad = RESOURCE_ID_WEATHER_LIGHT_SNOW;
      break;
    case 16: //snow
    case 18: //sleet
    case 41: //heavy snow
    case 43: //heavy snow
    case 46: //snow showers
      iconToLoad = RESOURCE_ID_WEATHER_HEAVY_SNOW;
      break;
    case 19: //dust
    case 20: //foggy
    case 21: //haze
    case 22: //smoky
    case 26: //cloudy
      iconToLoad = RESOURCE_ID_WEATHER_CLOUDY;
      break;
    case 27: //mostly cloudy (night)
    case 29: //partly cloudy (night)
      iconToLoad = RESOURCE_ID_WEATHER_PARTLY_CLOUDY_NIGHT;
      break;
    case 23: //blustery
    case 24: //windy
    case 25: //cold
    case 34: //fair (day)
    case 32: //sunny
    case 36: //hot
      iconToLoad = RESOURCE_ID_WEATHER_CLEAR_DAY;
      break;
    case 31: //clear (night)
    case 33: //fair (night)
      iconToLoad = RESOURCE_ID_WEATHER_CLEAR_NIGHT;
      break;
    case 28: //mostly cloudy (day)
    case 30: //partly cloudy (day)
    case 44: //partly cloudy
      iconToLoad = RESOURCE_ID_WEATHER_PARTLY_CLOUDY;
      break;
    default:
      iconToLoad = RESOURCE_ID_WEATHER_GENERIC;
      break;
  }

  return iconToLoad;
}

void Weather_setConditions(int conditionCode, bool isNight, int forecastCondition) {

  uint32_t currentWeatherIcon = getConditionIcon(conditionCode);
  uint32_t forecastWeatherIcon = getConditionIcon(forecastCondition);

  // ok, now load the new icon:
  gdraw_command_image_destroy(Weather_currentWeatherIcon);
  Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(currentWeatherIcon);

  gdraw_command_image_destroy(Weather_forecastWeatherIcon);
  Weather_forecastWeatherIcon = gdraw_command_image_create_with_resource(forecastWeatherIcon);

  Weather_weatherInfo.currentIconResourceID = currentWeatherIcon;
  Weather_weatherForecast.forecastIconResourceID = forecastWeatherIcon;
}

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
