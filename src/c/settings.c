#include "settings.h"
#include <pebble.h>

Settings settings;
DynamicSettings dynamicSettings;

void Settings_init() {
  Settings_loadFromStorage();

  // for anyone who already installed timestyle on an emery, reset
  // their large font setting, since the old "large" setting is
  // roughly equivalent to the new default, and the new "large"
  // setting is EVEN LARGER
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  if (!persist_exists(MIGRATION_EMERY_FONT_PERSIST_KEY)) {
    if (settings.useLargeFonts) {
      settings.useLargeFonts = false;
      Settings_saveToStorage();
    }
    persist_write_bool(MIGRATION_EMERY_FONT_PERSIST_KEY, true);
  }
#endif
}

void Settings_deinit() { Settings_saveToStorage(); }

void Settings_loadFromStorage() {
  // Set defaults
  settings.timeBgColor = GColorBlack;
  settings.sidebarTextColor = GColorBlack;

#ifdef PBL_COLOR
  settings.timeColor = GColorOrange;
  settings.sidebarColor = GColorOrange;
#else
  settings.timeColor = GColorWhite;
  settings.sidebarColor = GColorWhite;
#endif

  settings.vRegionEdge = REGION_EDGE_RIGHT;
  settings.hRegionEdge = REGION_EDGE_NONE;
  settings.hRegionStyle = HBAR_STYLE_INLINE;

#ifdef PBL_ROUND
  // round watches render the first two slots, one per arc
  settings.vRegionSlots[0] = WEATHER_CURRENT;
  settings.vRegionSlots[1] = DATE;
  settings.vRegionSlots[2] = EMPTY;
  settings.vRegionSlots[3] = EMPTY;
#elif VREGION_SLOT_CAP == 4
  // the last rendered slot pins to the bottom of the sidebar
  settings.vRegionSlots[0] = WEATHER_CURRENT;
  settings.vRegionSlots[1] = EMPTY;
  settings.vRegionSlots[2] = EMPTY;
  settings.vRegionSlots[3] = DATE;
#else
  settings.vRegionSlots[0] = WEATHER_CURRENT;
  settings.vRegionSlots[1] = EMPTY;
  settings.vRegionSlots[2] = DATE;
  settings.vRegionSlots[3] = EMPTY;
#endif

  settings.hRegionSlots[0] = BATTERY_METER;
  settings.hRegionSlots[1] = SECONDS;
  settings.hRegionSlots[2] = EMPTY;
  settings.hRegionSlots[3] = EMPTY;

  settings.activateDisconnectIcon = true;
  strncpy(settings.altclockName, "ALT", sizeof(settings.altclockName));
  settings.altclockOffset = 0;
  settings.decimalSeparator = '.';
  settings.showBatteryPct = true;
  settings.useWideSidebar = false;

  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
    settings.altclockName[sizeof(settings.altclockName) - 1] = '\0';
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  Settings_updateDynamicSettings();

  // We're limited to 256 bytes, so make sure that settings fits
  _Static_assert(sizeof(Settings) <= PERSIST_DATA_MAX_LENGTH,
                 "Warning: settings struct is too large!");
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Current settings size %d", sizeof(settings));

  // Write the data
  persist_write_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
  persist_write_int(SETTINGS_VERSION_PERSIST_KEY, CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings() {
  dynamicSettings.disableWeather = true;
  dynamicSettings.updateScreenEverySecond = false;
  dynamicSettings.enableBeats = false;
  dynamicSettings.enableAltTimeZone = false;

  // scan the widget slots that can actually render
#ifdef PBL_ROUND
  int vSlotCount = 2;
#else
  int vSlotCount = VREGION_SLOT_CAP;
#endif
  int hSlotCount = 0;

  if (settings.hRegionEdge != REGION_EDGE_NONE) {
    hSlotCount = (settings.hRegionStyle == HBAR_STYLE_STACKED)
                     ? HREGION_SLOT_CAP_STACKED
                     : HREGION_SLOT_CAP_INLINE;
  }

  for (int i = 0; i < vSlotCount + hSlotCount; i++) {
    SidebarWidgetType widget =
        (SidebarWidgetType)(i < vSlotCount
                                ? settings.vRegionSlots[i]
                                : settings.hRegionSlots[i - vSlotCount]);

    // if there are any weather widgets, enable weather checking
    if (widget == WEATHER_CURRENT ||
        widget == WEATHER_FORECAST_TODAY ||
        widget == WEATHER_UV_INDEX) {
      dynamicSettings.disableWeather = false;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if (widget == SECONDS) {
      dynamicSettings.updateScreenEverySecond = true;
    }

    // if any widget is "beats", enable the beats calculation
    if (widget == BEATS) {
      dynamicSettings.enableBeats = true;
    }

    // if any widget is "alt_time_zone", enable the alternative time calculation
    if (widget == ALT_TIME_ZONE) {
      dynamicSettings.enableAltTimeZone = true;
    }
  }

  // if the sidebar is black, use inverted colors for icons
  if (gcolor_equal(settings.sidebarColor, GColorBlack)) {
    dynamicSettings.iconFillColor = GColorBlack;
    dynamicSettings.iconStrokeColor = settings.sidebarTextColor;
  } else {
    dynamicSettings.iconFillColor = GColorWhite;
    dynamicSettings.iconStrokeColor = GColorBlack;
  }
}
