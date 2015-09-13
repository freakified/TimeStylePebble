#include "settings.h"

void Settings_init() {
  // first, check if we have any saved settings
  int settingsVersion = persist_read_int(SETTINGS_VERSION_KEY);

  if(settingsVersion == 2) {
    // if it's the old version of the settings migrate them
    Settings_migrateLegacySettings();
  }

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

  // load the rest of the settings, using default settings if none exist
  // all settings except colors automatically return "0" or "false" if
  // they haven't been set yet, so we don't need to check if they exist
  globalSettings.useMetric              = persist_read_bool(SETTING_USE_METRIC_KEY);
  globalSettings.sidebarOnLeft          = persist_read_bool(SETTING_SIDEBAR_LEFT_KEY);
  globalSettings.btVibe                 = persist_read_bool(SETTING_BT_VIBE_KEY);
  globalSettings.showBatteryLevel       = persist_read_bool(SETTING_SHOW_BATTERY_METER_KEY);
  globalSettings.languageId             = persist_read_int(SETTING_LANGUAGE_ID_KEY);
  globalSettings.showLeadingZero        = persist_read_int(SETTING_LEADING_ZERO_KEY);
  globalSettings.showBatteryPct         = persist_read_bool(SETTING_SHOW_BATTERY_PCT_KEY);
  globalSettings.disableWeather         = persist_read_bool(SETTING_DISABLE_WEATHER_KEY);
  globalSettings.clockFontId            = persist_read_bool(SETTING_CLOCK_FONT_ID_KEY);
  globalSettings.hourlyVibe             = persist_read_int(SETTING_HOURLY_VIBE_KEY);
  globalSettings.onlyShowBatteryWhenLow = persist_read_bool(SETTING_BATTERY_ONLY_WHEN_LOW_KEY);
  globalSettings.useLargeFonts          = persist_read_bool(SETTING_USE_LARGE_FONTS_KEY);
}

void Settings_saveToStorage() {
  // save settings to persistent storage
  persist_write_data(SETTING_TIME_COLOR_KEY,            &globalSettings.timeColor,        sizeof(GColor));
  persist_write_data(SETTING_TIME_BG_COLOR_KEY,         &globalSettings.timeBgColor,      sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_COLOR_KEY,         &globalSettings.sidebarColor,     sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_TEXT_COLOR_KEY,    &globalSettings.sidebarTextColor, sizeof(GColor));
  persist_write_bool(SETTING_USE_METRIC_KEY,            globalSettings.useMetric);
  persist_write_bool(SETTING_SIDEBAR_LEFT_KEY,          globalSettings.sidebarOnLeft);
  persist_write_bool(SETTING_BT_VIBE_KEY,               globalSettings.btVibe);
  persist_write_bool(SETTING_SHOW_BATTERY_METER_KEY,    globalSettings.showBatteryLevel);
  persist_write_int( SETTING_LANGUAGE_ID_KEY,           globalSettings.languageId);
  persist_write_int( SETTING_LEADING_ZERO_KEY,          globalSettings.showLeadingZero);
  persist_write_bool(SETTING_SHOW_BATTERY_PCT_KEY,      globalSettings.showBatteryPct);
  persist_write_bool(SETTING_DISABLE_WEATHER_KEY,       globalSettings.disableWeather);
  persist_write_bool(SETTING_CLOCK_FONT_ID_KEY,         globalSettings.clockFontId);
  persist_write_int( SETTING_HOURLY_VIBE_KEY,           globalSettings.hourlyVibe);
  persist_write_bool(SETTING_BATTERY_ONLY_WHEN_LOW_KEY, globalSettings.onlyShowBatteryWhenLow);
  persist_write_bool(SETTING_USE_LARGE_FONTS_KEY,       globalSettings.useLargeFonts);
  persist_write_int(SETTINGS_VERSION_KEY,               CURRENT_SETTINGS_VERSION);
}

void Settings_migrateLegacySettings() {
  Settings_Legacy oldSettings;

  // read the older settings
  persist_read_data(LEGACY_SETTINGS_PERSIST_KEY, &oldSettings, sizeof(Settings_Legacy));

  // write each setting into the new format
  persist_write_data(SETTING_TIME_COLOR_KEY,         &oldSettings.timeColor,        sizeof(GColor));
  persist_write_data(SETTING_TIME_BG_COLOR_KEY,      &oldSettings.timeBgColor,      sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_COLOR_KEY,      &oldSettings.sidebarColor,     sizeof(GColor));
  persist_write_data(SETTING_SIDEBAR_TEXT_COLOR_KEY, &oldSettings.sidebarTextColor, sizeof(GColor));
  persist_write_bool(SETTING_USE_METRIC_KEY,         oldSettings.useMetric);
  persist_write_bool(SETTING_SIDEBAR_LEFT_KEY,       !oldSettings.sidebarOnRight);
  persist_write_bool(SETTING_BT_VIBE_KEY,            oldSettings.btVibe);
  persist_write_bool(SETTING_SHOW_BATTERY_METER_KEY, oldSettings.showBatteryLevel);
  persist_write_int( SETTING_LANGUAGE_ID_KEY,        oldSettings.languageId);

  // delete the old settings data
  persist_delete(LEGACY_SETTINGS_PERSIST_KEY);
}
