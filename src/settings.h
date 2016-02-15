#pragma once
#include <pebble.h>
#include "sidebar_widgets/sidebar_widgets.h"

#define SETTINGS_VERSION_KEY 4

// settings "version" for app version 4.0
#define CURRENT_SETTINGS_VERSION 5

typedef struct {
  // color settings
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;

  // general settings
  uint8_t languageId;
  bool showLeadingZero;
  uint8_t clockFontId;

  // vibration settings
  bool btVibe;
  bool hourlyVibe;

  // sidebar settings
  SidebarWidgetType widgets[3];
  bool sidebarOnLeft;
  bool useLargeFonts;

  // weather widget settings
  bool useMetric;

  // battery meter widget settings
  bool showBatteryPct;

  // alt tz widget settings
  char altclockName[8];
  int altclockOffset;

  // health widget Settings
  bool healthUseDistance;
  bool healthUseRestfulSleep;
  char decimalSeparator;

  // dynamic settings (calculated based the currently-selected widgets)
  bool disableWeather;
  bool updateScreenEverySecond;
  bool enableAutoBatteryWidget;

  // TODO: these shouldn't be dynamic
  GColor iconFillColor;
  GColor iconStrokeColor;
} Settings;

extern Settings globalSettings;

// persistent storage keys for each setting

// color settings
#define SETTING_TIME_COLOR_KEY            17
#define SETTING_TIME_BG_COLOR_KEY         18
#define SETTING_SIDEBAR_COLOR_KEY         19
#define SETTING_SIDEBAR_TEXT_COLOR_KEY    20

// general settings
#define SETTING_LANGUAGE_ID_KEY           25
#define SETTING_LEADING_ZERO_KEY          10
#define SETTING_CLOCK_FONT_ID_KEY         13

// vibration settings
#define SETTING_BT_VIBE_KEY               23
#define SETTING_HOURLY_VIBE_KEY           14

// sidebar settings
#define SETTING_SIDEBAR_WIDGET0_KEY       26
#define SETTING_SIDEBAR_WIDGET1_KEY       27
#define SETTING_SIDEBAR_WIDGET2_KEY       28
#define SETTING_SIDEBAR_LEFT_KEY          22
#define SETTING_USE_LARGE_FONTS_KEY       16

// weather widget settings
#define SETTING_DISABLE_WEATHER_KEY       12
#define SETTING_USE_METRIC_KEY            21

// battery meter widget settings
#define SETTING_SHOW_BATTERY_PCT_KEY      11

// alt tz widget settings
#define SETTING_ALTCLOCK_NAME_KEY         30
#define SETTING_ALTCLOCK_OFFSET_KEY       31

// health widget settings
#define SETTING_HEALTH_USE_DISTANCE       32
#define SETTING_HEALTH_USE_RESTFUL_SLEEP  33
#define SETTING_HEALTH_USE_METRIC         35
#define SETTING_DECIMAL_SEPARATOR_KEY     34

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
