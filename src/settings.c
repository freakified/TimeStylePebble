#include <pebble.h>
#include "settings.h"

Settings globalSettings;

void Settings_init() {
  // first, check if we have any saved settings
  int settingsVersion = persist_read_int(SETTINGS_VERSION_KEY);

  // load all settings
  Settings_loadFromStorage();

  // for BW watches, reset colors to defaults
  #ifndef PBL_COLOR
    if(settingsVersion < CURRENT_SETTINGS_VERSION) {
      globalSettings.timeColor      = GColorWhite;
      globalSettings.sidebarColor   = GColorWhite;
      globalSettings.timeBgColor      = GColorBlack;
      globalSettings.sidebarTextColor = GColorBlack;
    }
  #endif
}

void Settings_deinit() {
  // write all settings to storage
  Settings_saveToStorage();
}

/*
 * Load the saved color settings, or if they don't exist load defaults
 */
void Settings_loadFromStorage() {
  if(persist_exists(SETTING_TIME_COLOR_KEY) && persist_exists(SETTING_TIME_BG_COLOR_KEY) &&
     persist_exists(SETTING_SIDEBAR_COLOR_KEY) && persist_exists(SETTING_SIDEBAR_TEXT_COLOR_KEY)) {

    // if the color data exists, load the colors
    persist_read_data(SETTING_TIME_COLOR_KEY,         &globalSettings.timeColor,        sizeof(GColor));
    persist_read_data(SETTING_TIME_BG_COLOR_KEY,      &globalSettings.timeBgColor,      sizeof(GColor));
    persist_read_data(SETTING_SIDEBAR_COLOR_KEY,      &globalSettings.sidebarColor,     sizeof(GColor));
    persist_read_data(SETTING_SIDEBAR_TEXT_COLOR_KEY, &globalSettings.sidebarTextColor, sizeof(GColor));
  } else {
    // otherwise, load the default colors
    globalSettings.timeBgColor      = GColorBlack;
    globalSettings.sidebarTextColor = GColorBlack;

    #ifdef PBL_COLOR
      globalSettings.timeColor      = GColorOrange;
      globalSettings.sidebarColor   = GColorOrange;
    #else
      globalSettings.timeColor      = GColorWhite;
      globalSettings.sidebarColor   = GColorWhite;
    #endif
  }

  // load widgets
  if(persist_exists(SETTING_SIDEBAR_WIDGET0_KEY)) {
    globalSettings.widgets[0] = persist_read_int(SETTING_SIDEBAR_WIDGET0_KEY);
    globalSettings.widgets[1] = persist_read_int(SETTING_SIDEBAR_WIDGET1_KEY);
    globalSettings.widgets[2] = persist_read_int(SETTING_SIDEBAR_WIDGET2_KEY);
  } else {
    // in the case of a new installation, set the default widgets
    globalSettings.widgets[0] = WEATHER_CURRENT;
    globalSettings.widgets[1] = EMPTY;
    globalSettings.widgets[2] = DATE;
  }

  if(persist_exists(SETTING_ALTCLOCK_NAME_KEY)) {
    persist_read_string(SETTING_ALTCLOCK_NAME_KEY, globalSettings.altclockName, sizeof(globalSettings.altclockName));
  } else {
    strncpy(globalSettings.altclockName, "ALT", sizeof(globalSettings.altclockName));
  }

  // load the rest of the settings, using default settings if none exist
  // all settings except colors automatically return "0" or "false" if
  // they haven't been set yet, so we don't need to check if they exist
  globalSettings.useMetric              = persist_read_bool(SETTING_USE_METRIC_KEY);
  globalSettings.sidebarOnLeft          = persist_read_bool(SETTING_SIDEBAR_LEFT_KEY);
  globalSettings.btVibe                 = persist_read_bool(SETTING_BT_VIBE_KEY);
  globalSettings.languageId             = persist_read_int(SETTING_LANGUAGE_ID_KEY);
  globalSettings.showLeadingZero        = persist_read_int(SETTING_LEADING_ZERO_KEY);
  globalSettings.showBatteryPct         = persist_read_bool(SETTING_SHOW_BATTERY_PCT_KEY);
  globalSettings.disableWeather         = persist_read_bool(SETTING_DISABLE_WEATHER_KEY);
  globalSettings.clockFontId            = persist_read_int(SETTING_CLOCK_FONT_ID_KEY);
  globalSettings.hourlyVibe             = persist_read_int(SETTING_HOURLY_VIBE_KEY);
  globalSettings.useLargeFonts          = persist_read_bool(SETTING_USE_LARGE_FONTS_KEY);
  globalSettings.altclockOffset         = persist_read_int(SETTING_ALTCLOCK_OFFSET_KEY);
  globalSettings.healthUseDistance      = persist_read_bool(SETTING_HEALTH_USE_DISTANCE);
  globalSettings.healthUseRestfulSleep  = persist_read_bool(SETTING_HEALTH_USE_RESTFUL_SLEEP);
  globalSettings.altclockOffset         = persist_read_int(SETTING_ALTCLOCK_OFFSET_KEY);

  if(persist_exists(SETTING_DECIMAL_SEPARATOR_KEY)) {
    globalSettings.decimalSeparator = '.';
  } else {
    globalSettings.decimalSeparator = (char)persist_read_int(SETTING_DECIMAL_SEPARATOR_KEY);
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  // ensure that the weather disabled setting is accurate before saving it
  Settings_updateDynamicSettings();

  // save settings to persistent storage
  persist_write_data(SETTING_TIME_COLOR_KEY,            &globalSettings.timeColor,        sizeof(GColor));
  persist_write_data(SETTING_TIME_BG_COLOR_KEY,         &globalSettings.timeBgColor,      sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_COLOR_KEY,         &globalSettings.sidebarColor,     sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_TEXT_COLOR_KEY,    &globalSettings.sidebarTextColor, sizeof(GColor));
  persist_write_bool(SETTING_USE_METRIC_KEY,            globalSettings.useMetric);
  persist_write_bool(SETTING_SIDEBAR_LEFT_KEY,          globalSettings.sidebarOnLeft);
  persist_write_bool(SETTING_BT_VIBE_KEY,               globalSettings.btVibe);
  persist_write_int( SETTING_LANGUAGE_ID_KEY,           globalSettings.languageId);
  persist_write_int( SETTING_LEADING_ZERO_KEY,          globalSettings.showLeadingZero);
  persist_write_bool(SETTING_SHOW_BATTERY_PCT_KEY,      globalSettings.showBatteryPct);
  persist_write_bool(SETTING_DISABLE_WEATHER_KEY,       globalSettings.disableWeather);
  persist_write_int(SETTING_CLOCK_FONT_ID_KEY,          globalSettings.clockFontId);
  persist_write_int( SETTING_HOURLY_VIBE_KEY,           globalSettings.hourlyVibe);
  persist_write_bool(SETTING_USE_LARGE_FONTS_KEY,       globalSettings.useLargeFonts);
  persist_write_int(SETTING_SIDEBAR_WIDGET0_KEY,        globalSettings.widgets[0]);
  persist_write_int(SETTING_SIDEBAR_WIDGET1_KEY,        globalSettings.widgets[1]);
  persist_write_int(SETTING_SIDEBAR_WIDGET2_KEY,        globalSettings.widgets[2]);
  persist_write_string(SETTING_ALTCLOCK_NAME_KEY,       globalSettings.altclockName);
  persist_write_int(SETTING_ALTCLOCK_OFFSET_KEY,        globalSettings.altclockOffset);
  persist_write_int(SETTING_DECIMAL_SEPARATOR_KEY, (int)globalSettings.decimalSeparator);

  persist_write_int(SETTINGS_VERSION_KEY,               CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings() {
  globalSettings.disableWeather = true;
  globalSettings.updateScreenEverySecond = false;
  globalSettings.enableAutoBatteryWidget = true;

  for(int i = 0; i < 3; i++) {
    // if there are any weather widgets, enable weather checking
    if(globalSettings.widgets[i] == WEATHER_CURRENT ||
       globalSettings.widgets[i] == WEATHER_FORECAST_TODAY) {

      globalSettings.disableWeather = false;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if(globalSettings.widgets[i] == SECONDS) {
      globalSettings.updateScreenEverySecond = true;
    }

    // if any widget is "battery", disable the automatic battery indication
    if(globalSettings.widgets[i] == BATTERY_METER) {
      globalSettings.enableAutoBatteryWidget = false;
    }
  }

  // temp: if the sidebar is black, use inverted colors for icons
  if(gcolor_equal(globalSettings.sidebarColor, GColorBlack)) {
    globalSettings.iconFillColor = GColorBlack;
    globalSettings.iconStrokeColor = GColorWhite;
  } else {
    globalSettings.iconFillColor = GColorWhite;
    globalSettings.iconStrokeColor = GColorBlack;
  }
}
