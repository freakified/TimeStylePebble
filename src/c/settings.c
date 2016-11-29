#include <pebble.h>
#include "settings.h"

Settings globalSettings;

void Settings_init() {
  // first, check if we have any saved settings
  int settingsVersion = persist_read_int(SETTINGS_VERSION_KEY);

  // load all settings
  Settings_loadFromStorage();
}

void Settings_deinit() {
  // write all settings to storage
  Settings_saveToStorage();
}

/*
 * Load the saved color settings, or if they don't exist load defaults
 */
void Settings_loadFromStorage() {
  // load the default colors
  globalSettings.timeBgColor      = GColorBlack;
  globalSettings.sidebarTextColor = GColorBlack;

  #ifdef PBL_COLOR
    globalSettings.timeColor      = GColorOrange;
    globalSettings.sidebarColor   = GColorOrange;
  #else
    globalSettings.timeColor      = GColorWhite;
    globalSettings.sidebarColor   = GColorLightGray;
  #endif

  // set the default widgets
  globalSettings.widgets[0] = PBL_IF_HEALTH_ELSE(HEALTH, BATTERY_METER);
  globalSettings.widgets[1] = EMPTY;
  globalSettings.widgets[2] = DATE;

  // set default disconnect icon activation
  globalSettings.activateDisconnectIcon = false; //TODO: Set to true

  strncpy(globalSettings.altclockName, "ALT", sizeof(globalSettings.altclockName));
  globalSettings.decimalSeparator = '.';

  globalSettings.showBatteryPct = true;

  int current_settings_version = persist_exists(SETTINGS_VERSION_KEY) ? persist_read_int(SETTINGS_VERSION_KEY) : -1;
  APP_LOG(APP_LOG_LEVEL_DEBUG,"current_settings_version: %d", current_settings_version);
  if( current_settings_version > 5 ) {
    // new settings format
    StoredSettings storedSettings;
    memset(&storedSettings,0,sizeof(StoredSettings));
    // if previous version settings are used than only first part of settings would be overwritten,
    // all the other fields will left filled with zeroes
    persist_read_data(SETTING_VERSION6_AND_HIGHER, &storedSettings, sizeof(StoredSettings));
    globalSettings.timeColor = storedSettings.timeColor;
    globalSettings.timeBgColor = storedSettings.timeBgColor;
    globalSettings.sidebarColor = storedSettings.sidebarColor;
    globalSettings.sidebarTextColor = storedSettings.sidebarTextColor;
    globalSettings.languageId = storedSettings.languageId;
    globalSettings.showLeadingZero = storedSettings.showLeadingZero;
    globalSettings.clockFontId = storedSettings.clockFontId;
    globalSettings.btVibe = storedSettings.btVibe;
    globalSettings.hourlyVibe = storedSettings.hourlyVibe;
    globalSettings.widgets[0] = storedSettings.widgets[0];
    globalSettings.widgets[1] = storedSettings.widgets[1];
    globalSettings.widgets[2] = storedSettings.widgets[2];
    globalSettings.sidebarOnLeft = storedSettings.sidebarOnLeft;
    globalSettings.useLargeFonts = storedSettings.useLargeFonts;
    globalSettings.useMetric = storedSettings.useMetric;
    globalSettings.showBatteryPct = storedSettings.showBatteryPct;
    globalSettings.disableAutobattery = storedSettings.disableAutobattery;
    globalSettings.healthUseDistance = storedSettings.healthUseDistance;
    globalSettings.healthUseRestfulSleep = storedSettings.healthUseRestfulSleep;
    globalSettings.decimalSeparator = storedSettings.decimalSeparator;
    memcpy(globalSettings.altclockName, storedSettings.altclockName, 8);
    globalSettings.altclockOffset = storedSettings.altclockOffset;
    globalSettings.activateDisconnectIcon = storedSettings.activateDisconnectIcon;
  } else if( current_settings_version >= 0 ) {
    // old settings format
    if(persist_exists(SETTING_TIME_COLOR_KEY) && persist_exists(SETTING_TIME_BG_COLOR_KEY) &&
       persist_exists(SETTING_SIDEBAR_COLOR_KEY) && persist_exists(SETTING_SIDEBAR_TEXT_COLOR_KEY)) {

      // if the color data exists, load the colors
      persist_read_data(SETTING_TIME_COLOR_KEY,         &globalSettings.timeColor,        sizeof(GColor));
      persist_read_data(SETTING_TIME_BG_COLOR_KEY,      &globalSettings.timeBgColor,      sizeof(GColor));
      persist_read_data(SETTING_SIDEBAR_COLOR_KEY,      &globalSettings.sidebarColor,     sizeof(GColor));
      persist_read_data(SETTING_SIDEBAR_TEXT_COLOR_KEY, &globalSettings.sidebarTextColor, sizeof(GColor));
    }

    // load widgets
    if(persist_exists(SETTING_SIDEBAR_WIDGET0_KEY)) {
      globalSettings.widgets[0] = persist_read_int(SETTING_SIDEBAR_WIDGET0_KEY);
      globalSettings.widgets[1] = persist_read_int(SETTING_SIDEBAR_WIDGET1_KEY);
      globalSettings.widgets[2] = persist_read_int(SETTING_SIDEBAR_WIDGET2_KEY);
    }

    if(persist_exists(SETTING_ALTCLOCK_NAME_KEY)) {
      persist_read_string(SETTING_ALTCLOCK_NAME_KEY, globalSettings.altclockName, sizeof(globalSettings.altclockName));
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
    globalSettings.disableAutobattery     = persist_read_bool(SETTING_DISABLE_AUTOBATTERY);
    globalSettings.disableWeather         = persist_read_bool(SETTING_DISABLE_WEATHER_KEY);
    globalSettings.clockFontId            = persist_read_int(SETTING_CLOCK_FONT_ID_KEY);
    globalSettings.hourlyVibe             = persist_read_int(SETTING_HOURLY_VIBE_KEY);
    globalSettings.useLargeFonts          = persist_read_bool(SETTING_USE_LARGE_FONTS_KEY);
    globalSettings.altclockOffset         = persist_read_int(SETTING_ALTCLOCK_OFFSET_KEY);
    globalSettings.healthUseDistance      = persist_read_bool(SETTING_HEALTH_USE_DISTANCE);
    globalSettings.healthUseRestfulSleep  = persist_read_bool(SETTING_HEALTH_USE_RESTFUL_SLEEP);

    if(persist_exists(SETTING_DECIMAL_SEPARATOR_KEY)) {
      globalSettings.decimalSeparator = (char)persist_read_int(SETTING_DECIMAL_SEPARATOR_KEY);
    }
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  // ensure that the weather disabled setting is accurate before saving it
  Settings_updateDynamicSettings();

  // save settings to compressed structure and to persistent storage
  StoredSettings storedSettings;
  // if previous version settings are used than only first part of settings would be overwrited
  // all the other fields will left filled with zeroes
  storedSettings.timeColor = globalSettings.timeColor;
  storedSettings.timeBgColor = globalSettings.timeBgColor;
  storedSettings.sidebarColor = globalSettings.sidebarColor;
  storedSettings.sidebarTextColor = globalSettings.sidebarTextColor;
  storedSettings.languageId = globalSettings.languageId;
  storedSettings.showLeadingZero = globalSettings.showLeadingZero;
  storedSettings.clockFontId = globalSettings.clockFontId;
  storedSettings.btVibe = globalSettings.btVibe;
  storedSettings.hourlyVibe = globalSettings.hourlyVibe;
  storedSettings.widgets[0] = globalSettings.widgets[0];
  storedSettings.widgets[1] = globalSettings.widgets[1];
  storedSettings.widgets[2] = globalSettings.widgets[2];
  storedSettings.sidebarOnLeft = globalSettings.sidebarOnLeft;
  storedSettings.useLargeFonts = globalSettings.useLargeFonts;
  storedSettings.useMetric = globalSettings.useMetric;
  storedSettings.showBatteryPct = globalSettings.showBatteryPct;
  storedSettings.disableAutobattery = globalSettings.disableAutobattery;
  storedSettings.healthUseDistance = globalSettings.healthUseDistance;
  storedSettings.healthUseRestfulSleep = globalSettings.healthUseRestfulSleep;
  storedSettings.decimalSeparator = globalSettings.decimalSeparator;
  memcpy(storedSettings.altclockName, globalSettings.altclockName, 8);
  storedSettings.altclockOffset = globalSettings.altclockOffset;
  storedSettings.activateDisconnectIcon = globalSettings.activateDisconnectIcon;

  persist_write_data(SETTING_VERSION6_AND_HIGHER, &storedSettings, sizeof(StoredSettings));
  persist_write_int(SETTINGS_VERSION_KEY, CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings() {
  globalSettings.disableWeather = true;
  globalSettings.updateScreenEverySecond = false;
  globalSettings.enableAutoBatteryWidget = true;

  for(int i = 0; i < 3; i++) {
    // if there are any weather widgets, enable weather checking
    // if(globalSettings.widgets[i] == WEATHER_CURRENT ||
    //    globalSettings.widgets[i] == WEATHER_FORECAST_TODAY) {
    if(globalSettings.widgets[i] == WEATHER_CURRENT) {
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
    globalSettings.iconStrokeColor = globalSettings.sidebarTextColor; // exciting
  } else {
    globalSettings.iconFillColor = GColorWhite;
    globalSettings.iconStrokeColor = GColorBlack;
  }
}
