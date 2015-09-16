#pragma once
#include <pebble.h>

#define LEGACY_SETTINGS_PERSIST_KEY 3
#define SETTINGS_VERSION_KEY 4

// settings "version" for app version 3.6
#define CURRENT_SETTINGS_VERSION 3

// legacy settings struct for settings migration
// this will be removed once most installs have migrated
typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;
  bool useMetric;
  bool sidebarOnRight;
  bool btVibe;
  bool showBatteryLevel;
  uint8_t languageId;
} Settings_Legacy;

typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;
  bool useMetric;
  bool sidebarOnLeft;
  bool btVibe;
  bool showBatteryLevel;
  uint8_t languageId;
  bool showLeadingZero;
  bool showBatteryPct;
  bool disableWeather;
  bool clockFontId;
  bool hourlyVibe;
  bool onlyShowBatteryWhenLow;
  bool useLargeFonts;
} Settings;

extern Settings globalSettings;

// as of version 3.6, instead of doing more crazy migrations,
// all settings get their own keys
// some IDs are not quite in order since they were migrated from the old "struct" format
#define SETTING_TIME_COLOR_KEY            17
#define SETTING_TIME_BG_COLOR_KEY         18
#define SETTING_SIDEBAR_COLOR_KEY         19
#define SETTING_SIDEBAR_TEXT_COLOR_KEY    20
#define SETTING_USE_METRIC_KEY            21
#define SETTING_SIDEBAR_LEFT_KEY          22
#define SETTING_BT_VIBE_KEY               23
#define SETTING_SHOW_BATTERY_METER_KEY    24
#define SETTING_LANGUAGE_ID_KEY           25
#define SETTING_LEADING_ZERO_KEY          10
#define SETTING_SHOW_BATTERY_PCT_KEY      11
#define SETTING_DISABLE_WEATHER_KEY       12
#define SETTING_CLOCK_FONT_ID_KEY         13
#define SETTING_HOURLY_VIBE_KEY           14
#define SETTING_BATTERY_ONLY_WHEN_LOW_KEY 15
#define SETTING_USE_LARGE_FONTS_KEY       16

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_migrateLegacySettings();
