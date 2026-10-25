#pragma once
#include <pebble.h>
#include "sidebar_layout.h"
#include "sidebar_widgets.h"

#define CURRENT_SETTINGS_VERSION 18

// persistent storage keys
#define SETTINGS_PERSIST_KEY 100
#define SETTINGS_VERSION_PERSIST_KEY 4

// the settings version every master (7.x) release saved; the only older
// shape load migrates from (see settings.c)
#define SETTINGS_VERSION_MASTER 7

// clockFontId picks the typeface; hoursBold / minutesBold pick each
// line's weight independently
#define FONT_SETTING_AVENIR   0
#define FONT_SETTING_LECO     1
#define FONT_SETTING_GARAMOND 5

// master's ids that baked Avenir's weights into the font id; only ever
// found in master's persisted settings, which load migrates
#define FONT_SETTING_LEGACY_BOLD   2
#define FONT_SETTING_LEGACY_BOLD_H 3
#define FONT_SETTING_LEGACY_BOLD_M 4

typedef enum {
  NO_VIBE = 0,
  VIBE_EVERY_HOUR = 1,
  VIBE_EVERY_HALF_HOUR = 2
} VibeIntervalType;

// Settings struct -- note, all new settings should ALWAYS be added to the bottom
typedef struct {
  // color settings (v15: dropped the sidebar and bottom bar text colors;
  // text and icon colors are now derived from these three)
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;

  // general settings
  uint8_t languageId;
  bool showLeadingZero;
  uint8_t clockFontId;

  // vibration settings
  bool btVibe;
  VibeIntervalType hourlyVibe;

  // sidebar settings
  bool useLargeFonts;

  // weather widget settings
  bool useMetric;

  // world time widget settings (offset is minutes from UTC; the phone
  // recomputes it from the selected city's DST rules whenever it talks
  // to the watch)
  char altclockName[8];
  int altclockOffset;

  // health widget Settings
  bool healthUseRestfulSleep;
  char decimalSeparator;

  // sidebar layout settings
  // the layout only picks the sidebar's side: there is always a sidebar,
  // and the bottom bar is on exactly when hRegionSlots holds a widget
  // sidebar width: the "wide" sidebar trades clock room for widgets that
  // can show their full form (a world time's HH:MM, an unabbreviated
  // step count) instead of falling back to a compact one
  bool useWideSidebar;
  uint8_t layoutId; // SidebarLayoutId
  uint8_t vRegionSlots[MAX_REGION_SLOTS]; // SidebarWidgetType values
  uint8_t hRegionSlots[HREGION_SLOT_MAX];

  // second world time widget (same semantics as altclockName/Offset)
  char altclock2Name[8];
  int altclock2Offset;

  // the local time zone's current offset from UTC in minutes, sent by
  // the phone (-getTimezoneOffset()); world time = local + (city - local).
  // tm_gmtoff can't be trusted here -- its sign convention varies
  int localUtcOffset;

  // status icon settings (v14: replaced the v13 disableAutobattery /
  // activateDisconnectIcon fields); when enabled, a corner icon shows
  // disconnection or quiet time only while active -- unless
  // statusShowConnection also shows "connected" (mini-battery removed
  // for now)
  bool enableStatusBar;
  bool statusShowConnection;

  // round only: normally just one arc is filled with the sidebar color
  // and the other blends into the watchface, which is what layoutId
  // picks between. This fills both, the way every arc used to look.
  bool colorBothArcs;

  // clock digit weights: bold or light, per line (v17)
  bool hoursBold;
  bool minutesBold;

  // widget parts the user has hidden, one bit per SidebarWidgetType (v18).
  // The "marker" is the part that names the widget -- its icon, its
  // label, or the date's weekday; the "detail" is a second part some
  // widgets can drop instead (battery percentage, weather temperatures,
  // the date's month). sidebar_widgets.c decides which bits apply.
  uint32_t hideWidgetMarker;
  uint32_t hideWidgetDetail;
} Settings;

// Dynamic settings (calculated at runtime based on currently-selected widgets)
typedef struct {
  bool disableWeather;
  bool updateScreenEverySecond;
  bool enableBeats;
  bool enableAltTimeZone;
  bool enableHeartRate;

  // whether the watch should periodically ping the phone (weather
  // updates, and world-time DST offset refreshes)
  bool enablePhonePolling;

  SidebarPalette sidebarPalette;     // "standard" region appearance
  SidebarPalette darkSidebarPalette; // "dark" region appearance
} DynamicSettings;

extern Settings settings;
extern DynamicSettings dynamicSettings;

void Settings_init();
void Settings_loadFromStorage();
void Settings_saveToStorage();
void Settings_updateDynamicSettings();
