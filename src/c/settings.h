#pragma once
#include <pebble.h>
#include "sidebar_widgets.h"

#define SETTINGS_VERSION_KEY 4

// settings "version" for app version 4.0
#define CURRENT_SETTINGS_VERSION 6

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
  int hourlyVibe;

  // sidebar settings
  SidebarWidgetType widgets[3];
  bool sidebarOnLeft;
  bool useLargeFonts;
  bool activateDisconnectIcon;
  
  // weather widget settings
  bool useMetric;

  // battery meter widget settings
  bool showBatteryPct;
  bool disableAutobattery;

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


// !! all future settings should be added to the bottom of this structure
// !! do not remove fields from this structure, it will lead to unexpected behaviour
typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;

  // general settings
  uint8_t languageId;
  uint8_t showLeadingZero:1;
  uint8_t clockFontId:7;

  // vibration settings
  uint8_t btVibe:1;
  int8_t hourlyVibe:7;

  // sidebar settings
  uint8_t widgets[3];
  uint8_t sidebarOnLeft:1;
  uint8_t useLargeFonts:1;

  // weather widget settings
  uint8_t useMetric:1;

  // battery meter widget settings
  uint8_t showBatteryPct:1;
  uint8_t disableAutobattery:1;

  // health widget Settings
  uint8_t healthUseDistance:1;
  uint8_t healthUseRestfulSleep:1;
  char decimalSeparator;

  // alt tz widget settings
  char altclockName[8];
  int8_t altclockOffset;
} StoredSettings;

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
#define SETTING_DISABLE_AUTOBATTERY       36

// alt tz widget settings
#define SETTING_ALTCLOCK_NAME_KEY         30
#define SETTING_ALTCLOCK_OFFSET_KEY       31

// health widget settings
#define SETTING_HEALTH_USE_DISTANCE       32
#define SETTING_HEALTH_USE_RESTFUL_SLEEP  33
#define SETTING_HEALTH_USE_METRIC         35
#define SETTING_DECIMAL_SEPARATOR_KEY     34

// key for all the settings for versions 6 and higher
#define SETTING_VERSION6_AND_HIGHER       100

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
