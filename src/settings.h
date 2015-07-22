#pragma once
#include <pebble.h>

#define SETTINGS_PERSIST_KEY 3
#define SETTINGS_VERSION_KEY 4

// settings for app version 1.2
#define CURRENT_SETTINGS_VERSION 2

// language id
#define LANGUAGE_EN 0

typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  bool useMetric;
  bool sidebarOnRight;
} Settings_v0;

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
} Settings;

Settings globalSettings;

// instead of doing more crazy migrations, henceforth new settings will get
// their own keys
#define SETTING_LEADING_ZERO_KEY 10
bool Settings_showLeadingZero;

#define SETTING_SHOW_BATTERY_PCT_KEY 11
bool Settings_showBatteryPct;

#define SETTING_DISABLE_WEATHER_KEY 12
bool Settings_disableWeather;

void Settings_init();
void Settings_loadAllDefaults();
void Settings_loadV2Defaults();
void Settings_deinit();
