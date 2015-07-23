#include "weather.h"

void Weather_setCondition(int conditionCode, bool isNight) {
  int generalCondition = conditionCode / 100;

  uint32_t iconToLoad;

  switch(generalCondition) {
    case 2: //thunderstorm
      iconToLoad = RESOURCE_ID_WEATHER_THUNDERSTORM;
      break;
    case 3: //drizzle
      iconToLoad = RESOURCE_ID_WEATHER_LIGHT_RAIN;
      break;
    case 5: //rain
      if(conditionCode == 500) {
        iconToLoad = RESOURCE_ID_WEATHER_LIGHT_RAIN;
      } else if(conditionCode < 505) {
        iconToLoad = RESOURCE_ID_WEATHER_HEAVY_RAIN;
      } else if(conditionCode == 511) {
        iconToLoad = RESOURCE_ID_WEATHER_RAINING_AND_SNOWING;
      } else {
        iconToLoad = RESOURCE_ID_WEATHER_LIGHT_RAIN;
      }
      break;
    case 6: //snow
      if(conditionCode == 600 || conditionCode == 620) {
        iconToLoad = RESOURCE_ID_WEATHER_LIGHT_SNOW;
      } else if(conditionCode > 610 && conditionCode < 620) {
        iconToLoad = RESOURCE_ID_WEATHER_RAINING_AND_SNOWING;
      } else {
        iconToLoad = RESOURCE_ID_WEATHER_HEAVY_SNOW;
      }
      break;
    case 7: // fog, dust, etc
      iconToLoad = RESOURCE_ID_WEATHER_CLOUDY;
      break;
    case 8: // clouds
      if(conditionCode == 800) {
        iconToLoad = (!isNight) ? RESOURCE_ID_WEATHER_CLEAR_DAY : RESOURCE_ID_WEATHER_CLEAR_NIGHT;
      } else if(conditionCode < 803) {
        iconToLoad = (!isNight) ? RESOURCE_ID_WEATHER_PARTLY_CLOUDY : RESOURCE_ID_WEATHER_PARTLY_CLOUDY_NIGHT;
      } else {
        iconToLoad = RESOURCE_ID_WEATHER_CLOUDY;
      }
      break;
    default: // hack: let's just treat the rest as clear
      iconToLoad = (!isNight) ? RESOURCE_ID_WEATHER_CLEAR_DAY : RESOURCE_ID_WEATHER_CLEAR_NIGHT;
      break;
  }

  // ok, now load the new icon:
  #ifdef PBL_COLOR
    GDrawCommandImage* oldImage = Weather_currentWeatherIcon;
    Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(iconToLoad);
    gdraw_command_image_destroy(oldImage);
  #else
    GBitmap* oldImage = Weather_currentWeatherIcon;
    Weather_currentWeatherIcon = gbitmap_create_with_resource(iconToLoad);
    gbitmap_destroy(oldImage);
  #endif

  Weather_weatherInfo.currentIconResourceID = iconToLoad;
}

void Weather_init() {
  // if possible, load weather data from persistent storage

  if (persist_exists(WEATHERINFO_PERSIST_KEY)) {
    WeatherInfo w;
    persist_read_data(WEATHERINFO_PERSIST_KEY, &w, sizeof(WeatherInfo));

    Weather_weatherInfo = w;

    #ifdef PBL_COLOR
      Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(w.currentIconResourceID);
    #else
      Weather_currentWeatherIcon = gbitmap_create_with_resource(w.currentIconResourceID);
    #endif

  } else {
    // otherwise, use null data
    Weather_currentWeatherIcon = NULL;
    Weather_weatherInfo.currentTemp = INT32_MIN;
  }
}

void Weather_deinit() {
  // save weather data to persistent storage
  persist_write_data(WEATHERINFO_PERSIST_KEY, &Weather_weatherInfo, sizeof(WeatherInfo));

  // free memory
  #ifdef PBL_COLOR
    gdraw_command_image_destroy(Weather_currentWeatherIcon);
  #else
    gbitmap_destroy(Weather_currentWeatherIcon);
  #endif
}
