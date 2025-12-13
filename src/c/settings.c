#include <pebble.h>
#include "settings.h"

Settings settings;
DynamicSettings dynamicSettings;

void Settings_init() {
  Settings_loadFromStorage();
}

void Settings_deinit() {
  Settings_saveToStorage();
}

void Settings_loadFromStorage() {
  // Set defaults
  settings.timeBgColor = GColorBlack;
  settings.sidebarTextColor = GColorBlack;

#ifdef PBL_COLOR
  settings.timeColor = GColorOrange;
  settings.sidebarColor = GColorOrange;
#else
  settings.timeColor = GColorWhite;
  settings.sidebarColor = GColorWhite;
#endif

  settings.widgets[0] = WEATHER_CURRENT;
  settings.widgets[1] = EMPTY;
  settings.widgets[2] = DATE;

  settings.activateDisconnectIcon = true;
  strncpy(settings.altclockName, "ALT", sizeof(settings.altclockName));
  settings.altclockOffset = 0;
  settings.decimalSeparator = '.';
  settings.showBatteryPct = true;

  // to correct settings migration bug (settings key v6), we must do another migration (nooooooooooo)
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    int version = 0;

    if (persist_exists(SETTINGS_VERSION_PERSIST_KEY)) {
      version = persist_read_int(SETTINGS_VERSION_PERSIST_KEY);

      if (version < CURRENT_SETTINGS_VERSION) {
        // v6 settings: load via memcpy
        LegacyStoredSettings s;
        persist_read_data(SETTINGS_PERSIST_KEY, &s, sizeof(s));
        memcpy(&settings, &s, sizeof(LegacyStoredSettings));

        // re-save in new v7 format
        Settings_saveToStorage();
      } else {
        // v7 settings: load directly via single persist read
        persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
        settings.altclockName[sizeof(settings.altclockName)-1] = '\0';
      }
    }
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  Settings_updateDynamicSettings();

  // We're limited to 256 bytes, so make sure that settings fits
  _Static_assert(sizeof(Settings) <= PERSIST_DATA_MAX_LENGTH, "Warning: settings struct is too large!");
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Current settings size %d", sizeof(settings));

  // Write the data
  persist_write_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
  persist_write_int(SETTINGS_VERSION_PERSIST_KEY, CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings() {
  dynamicSettings.disableWeather = false;
  dynamicSettings.enableApparentTemperature = false;
  dynamicSettings.updateScreenEverySecond = false;
  dynamicSettings.enableAutoBatteryWidget = true;
  dynamicSettings.enableBeats = false;
  dynamicSettings.enableAltTimeZone = false;

  for(int i = 0; i < 3; i++) {
    // if there are any weather widgets, enable weather checking
    if(settings.widgets[i] == WEATHER_CURRENT ||
       settings.widgets[i] == WEATHER_FORECAST_TODAY ||
       settings.widgets[i] == WEATHER_UV_INDEX) {
      dynamicSettings.disableWeather = false;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if(settings.widgets[i] == SECONDS) {
      dynamicSettings.updateScreenEverySecond = true;
    }

    // if any widget is "battery", disable the automatic battery indication
    if(settings.widgets[i] == BATTERY_METER) {
      dynamicSettings.enableAutoBatteryWidget = false;
    }

    // if any widget is "beats", enable the beats calculation
    if(settings.widgets[i] == BEATS) {
      dynamicSettings.enableBeats = true;
    }

    // if any widget is "alt_time_zone", enable the alternative time calculation
    if(settings.widgets[i] == ALT_TIME_ZONE) {
      dynamicSettings.enableAltTimeZone = true;
    }
  }

  // if the sidebar is black, use inverted colors for icons
  if(gcolor_equal(settings.sidebarColor, GColorBlack)) {
    dynamicSettings.iconFillColor = GColorBlack;
    dynamicSettings.iconStrokeColor = settings.sidebarTextColor;
  } else {
    dynamicSettings.iconFillColor = GColorWhite;
    dynamicSettings.iconStrokeColor = GColorBlack;
  }
}
