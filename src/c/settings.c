#include "settings.h"
#include <pebble.h>

Settings settings;
DynamicSettings dynamicSettings;

void Settings_init() {
  Settings_loadFromStorage();

  // for anyone who already installed timestyle on an emery, reset
  // their large font setting, since the old "large" setting is
  // roughly equivalent to the new default, and the new "large"
  // setting is EVEN LARGER
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  if (!persist_exists(MIGRATION_EMERY_FONT_PERSIST_KEY)) {
    if (settings.useLargeFonts) {
      settings.useLargeFonts = false;
      Settings_saveToStorage();
    }
    persist_write_bool(MIGRATION_EMERY_FONT_PERSIST_KEY, true);
  }
#endif
}

void Settings_deinit() { Settings_saveToStorage(); }

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
  settings.useWideSidebar = false;
  settings.useBottomBar = false;
  settings.bottomBarWidgets[0] = BATTERY_METER;
  settings.bottomBarWidgets[1] = SECONDS;

  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
    settings.altclockName[sizeof(settings.altclockName) - 1] = '\0';
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  Settings_updateDynamicSettings();

  // We're limited to 256 bytes, so make sure that settings fits
  _Static_assert(sizeof(Settings) <= PERSIST_DATA_MAX_LENGTH,
                 "Warning: settings struct is too large!");
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Current settings size %d", sizeof(settings));

  // Write the data
  persist_write_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
  persist_write_int(SETTINGS_VERSION_PERSIST_KEY, CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings() {
  dynamicSettings.disableWeather = true;
  dynamicSettings.updateScreenEverySecond = false;
  dynamicSettings.enableAutoBatteryWidget = true;
  dynamicSettings.enableBeats = false;
  dynamicSettings.enableAltTimeZone = false;

  int widgetCount = settings.useBottomBar ? 5 : 3;

  for (int i = 0; i < widgetCount; i++) {
    SidebarWidgetType widget = i < 3 ? settings.widgets[i]
                                     : settings.bottomBarWidgets[i - 3];

    // if there are any weather widgets, enable weather checking
    if (widget == WEATHER_CURRENT ||
        widget == WEATHER_FORECAST_TODAY ||
        widget == WEATHER_UV_INDEX) {
      dynamicSettings.disableWeather = false;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if (widget == SECONDS) {
      dynamicSettings.updateScreenEverySecond = true;
    }

    // if any widget is "battery", disable the automatic battery indication
    if (widget == BATTERY_METER) {
      dynamicSettings.enableAutoBatteryWidget = false;
    }

    // if any widget is "beats", enable the beats calculation
    if (widget == BEATS) {
      dynamicSettings.enableBeats = true;
    }

    // if any widget is "alt_time_zone", enable the alternative time calculation
    if (widget == ALT_TIME_ZONE) {
      dynamicSettings.enableAltTimeZone = true;
    }
  }

  // if the sidebar is black, use inverted colors for icons
  if (gcolor_equal(settings.sidebarColor, GColorBlack)) {
    dynamicSettings.iconFillColor = GColorBlack;
    dynamicSettings.iconStrokeColor = settings.sidebarTextColor;
  } else {
    dynamicSettings.iconFillColor = GColorWhite;
    dynamicSettings.iconStrokeColor = GColorBlack;
  }
}
