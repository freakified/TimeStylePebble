#include <pebble.h>
#include "weather.h"

WeatherInfo Weather_weatherInfo;

GDrawCommandImage* Weather_currentWeatherIcon;
GDrawCommandImage* Weather_forecastWeatherIcon;

static const uint32_t s_conditionIcons[] = {
  [CLEAR_DAY]           = RESOURCE_ID_WEATHER_CLEAR_DAY,
  [CLEAR_NIGHT]         = RESOURCE_ID_WEATHER_CLEAR_NIGHT,
  [CLOUDY_DAY]          = RESOURCE_ID_WEATHER_CLOUDY,
  [HEAVY_RAIN]          = RESOURCE_ID_WEATHER_HEAVY_RAIN,
  [HEAVY_SNOW]          = RESOURCE_ID_WEATHER_HEAVY_SNOW,
  [LIGHT_RAIN]          = RESOURCE_ID_WEATHER_LIGHT_RAIN,
  [LIGHT_SNOW]          = RESOURCE_ID_WEATHER_LIGHT_SNOW,
  [PARTLY_CLOUDY_NIGHT] = RESOURCE_ID_WEATHER_PARTLY_CLOUDY_NIGHT,
  [PARTLY_CLOUDY]       = RESOURCE_ID_WEATHER_PARTLY_CLOUDY,
  [RAINING_AND_SNOWING] = RESOURCE_ID_WEATHER_RAINING_AND_SNOWING,
  [THUNDERSTORM]        = RESOURCE_ID_WEATHER_THUNDERSTORM,
  [WEATHER_GENERIC]     = RESOURCE_ID_WEATHER_GENERIC,
};

// swaps *icon for the given condition's icon (NULL for no condition)
static void load_condition_icon(GDrawCommandImage** icon, int conditionCode) {
  gdraw_command_image_destroy(*icon);
  *icon = NULL;

  if (conditionCode < 0) {
    return;
  }

  uint32_t resourceId = (conditionCode < (int)ARRAY_LENGTH(s_conditionIcons))
                            ? s_conditionIcons[conditionCode]
                            : RESOURCE_ID_WEATHER_GENERIC;
  *icon = gdraw_command_image_create_with_resource(resourceId);
}

void Weather_setCurrentCondition(int conditionCode) {
  Weather_weatherInfo.currentCondition = conditionCode;
  load_condition_icon(&Weather_currentWeatherIcon, conditionCode);
}

void Weather_setForecastCondition(int conditionCode) {
  Weather_weatherInfo.forecastCondition = conditionCode;
  load_condition_icon(&Weather_forecastWeatherIcon, conditionCode);
}

void Weather_init() {
  // initialize weather with default values
  Weather_weatherInfo = (WeatherInfo){
    .currentTemp = INT32_MIN,
    .currentCondition = -1,
    .currentUVIndex = INT32_MIN,
    .todaysHighTemp = INT32_MIN,
    .todaysLowTemp = INT32_MIN,
    .forecastCondition = -1,
    .precipChance = INT32_MIN,
    .sunriseMinute = INT32_MIN,
    .sunsetMinute = INT32_MIN,
  };

  if (persist_exists(WEATHER_LEGACY_PERSIST_KEY)) {
    persist_delete(WEATHER_LEGACY_PERSIST_KEY);
  }

  // if possible, load weather data from persistent storage
  if (persist_exists(WEATHER_PERSIST_KEY)) {
    persist_read_data(WEATHER_PERSIST_KEY, &Weather_weatherInfo,
                      sizeof(Weather_weatherInfo));
  }

  load_condition_icon(&Weather_currentWeatherIcon,
                      Weather_weatherInfo.currentCondition);
  load_condition_icon(&Weather_forecastWeatherIcon,
                      Weather_weatherInfo.forecastCondition);
}

// called whenever new weather arrives, which is the only time it changes
void Weather_saveData() {
  persist_write_data(WEATHER_PERSIST_KEY, &Weather_weatherInfo, sizeof(WeatherInfo));
}

void Weather_deinit() {
  gdraw_command_image_destroy(Weather_currentWeatherIcon);
  gdraw_command_image_destroy(Weather_forecastWeatherIcon);
}
