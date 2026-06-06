#pragma once
#include <pebble.h>
#include "sidebar_widgets.h"

#define CURRENT_SETTINGS_VERSION 8

// persistent storage keys
#define SETTINGS_PERSIST_KEY 100
#define SETTINGS_VERSION_PERSIST_KEY 4
#define MIGRATION_EMERY_FONT_PERSIST_KEY 200

#define FONT_SETTING_DEFAULT 0
#define FONT_SETTING_LECO    1
#define FONT_SETTING_BOLD    2
#define FONT_SETTING_BOLD_H  3
#define FONT_SETTING_BOLD_M  4

typedef enum {
  NO_VIBE = 0,
  VIBE_EVERY_HOUR = 1,
  VIBE_EVERY_HALF_HOUR = 2
} VibeIntervalType;

// Settings struct -- note, all new settings should ALWAYS be added to the bottom
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
  VibeIntervalType hourlyVibe;

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

  // sidebar layout settings
  bool useWideSidebar;

  // date widget settings
  bool usePlainDateBG;
} Settings;

// Dynamic settings (calculated at runtime based on currently-selected widgets)
typedef struct {
  bool disableWeather;
  bool updateScreenEverySecond;
  bool enableAutoBatteryWidget;
  bool enableBeats;
  bool enableAltTimeZone;
  
  GColor iconFillColor;
  GColor iconStrokeColor;
} DynamicSettings;

extern Settings settings;
extern DynamicSettings dynamicSettings;

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
