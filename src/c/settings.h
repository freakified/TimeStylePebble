#pragma once
#include <pebble.h>
#include "sidebar_layout.h"
#include "sidebar_widgets.h"

#define CURRENT_SETTINGS_VERSION 13

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
  bool useLargeFonts;
  bool activateDisconnectIcon; // dormant: for the future status widget
  
  // weather widget settings
  bool useMetric;

  // battery meter widget settings
  bool disableAutobattery;

  // world time widget settings (offset is minutes from UTC; the phone
  // recomputes it from the selected city's DST rules whenever it talks
  // to the watch)
  char altclockName[8];
  int altclockOffset;

  // health widget Settings
  bool healthUseDistance;
  bool healthUseRestfulSleep;
  char decimalSeparator;

  // sidebar layout settings (v13: replaced the v12 vRegionEdge /
  // hRegionEdge / hRegionStyle fields with a single layout preset; old
  // persisted blobs are NOT translated yet -- a migration system for
  // pre-v13 settings comes later)
  // "seamless" sidebar: regions draw on the watchface background (the
  // "dark" appearance) and the vertical sidebar widens
  bool useSeamlessSidebar;
  uint8_t layoutId; // SidebarLayoutId
  uint8_t vRegionSlots[MAX_REGION_SLOTS]; // SidebarWidgetType values
  uint8_t hRegionSlots[MAX_REGION_SLOTS];

  // text color for "dark" sidebar regions (the bottom bar in two-sidebar
  // layouts, which draws on the watchface background color)
  GColor altSidebarTextColor;

  // second world time widget (same semantics as altclockName/Offset)
  char altclock2Name[8];
  int altclock2Offset;

  // the local time zone's current offset from UTC in minutes, sent by
  // the phone (-getTimezoneOffset()); world time = local + (city - local).
  // tm_gmtoff can't be trusted here -- its sign convention varies
  int localUtcOffset;
} Settings;

// Dynamic settings (calculated at runtime based on currently-selected widgets)
typedef struct {
  bool disableWeather;
  bool updateScreenEverySecond;
  bool enableBeats;
  bool enableAltTimeZone;

  // whether the watch should periodically ping the phone (weather
  // updates, and world-time DST offset refreshes)
  bool enablePhonePolling;

  SidebarPalette sidebarPalette;     // "standard" region appearance
  SidebarPalette darkSidebarPalette; // "dark" region appearance
} DynamicSettings;

extern Settings settings;
extern DynamicSettings dynamicSettings;

void Settings_init();
void Settings_deinit();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
