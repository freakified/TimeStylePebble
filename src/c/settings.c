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
  settings.altSidebarTextColor = GColorWhite;

#ifdef PBL_COLOR
  settings.timeColor = GColorOrange;
  settings.sidebarColor = GColorOrange;
#else
  settings.timeColor = GColorWhite;
  settings.sidebarColor = GColorWhite;
#endif

  settings.layoutId = LAYOUT_SIDEBAR_RIGHT;

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
  strncpy(settings.altclockName, "UTC", sizeof(settings.altclockName));
  settings.altclockOffset = 0;
  strncpy(settings.altclock2Name, "TYO", sizeof(settings.altclock2Name));
  settings.altclock2Offset = 540;
  settings.localUtcOffset = 0;
  settings.decimalSeparator = '.';
  settings.showBatteryPct = true;
  settings.useSeamlessSidebar = false;

  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
    settings.altclockName[sizeof(settings.altclockName) - 1] = '\0';
    settings.altclock2Name[sizeof(settings.altclock2Name) - 1] = '\0';
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
  dynamicSettings.enablePhonePolling = false;

  // scan the widget slots that can actually render
#ifdef PBL_ROUND
  int vSlotCount = 2;
#else
  int vSlotCount =
      (SidebarLayout_verticalEdge(settings.layoutId) == REGION_EDGE_NONE)
          ? 0
          : VREGION_SLOT_CAP;
#endif
  int hSlotCount = 0;

  if (SidebarLayout_horizontalEdge(settings.layoutId) != REGION_EDGE_NONE) {
    hSlotCount = (SidebarLayout_hBarStyle(settings.layoutId) == HBAR_STYLE_STACKED)
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
        widget == WEATHER_UV_INDEX ||
        widget == NEXT_SUN_EVENT ||
        widget == WEATHER_WIND ||
        widget == WEATHER_PRECIP_CHANCE) {
      dynamicSettings.disableWeather = false;
      dynamicSettings.enablePhonePolling = true;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if (widget == SECONDS) {
      dynamicSettings.updateScreenEverySecond = true;
    }

    // if any widget is "beats", enable the beats calculation
    if (widget == BEATS) {
      dynamicSettings.enableBeats = true;
    }

    // world time widgets need the shifted-clock calculation, plus periodic
    // phone contact so the city's UTC offset tracks DST changes
    if (widget == WORLD_TIME_1 || widget == WORLD_TIME_2) {
      dynamicSettings.enableAltTimeZone = true;
      dynamicSettings.enablePhonePolling = true;
    }
  }

  dynamicSettings.sidebarPalette.background = settings.sidebarColor;
  dynamicSettings.sidebarPalette.text = settings.sidebarTextColor;

  // if the sidebar is black, use inverted colors for icons
  if (gcolor_equal(settings.sidebarColor, GColorBlack)) {
    dynamicSettings.sidebarPalette.iconFill = GColorBlack;
    dynamicSettings.sidebarPalette.iconStroke = settings.sidebarTextColor;
  } else {
    dynamicSettings.sidebarPalette.iconFill = GColorWhite;
    dynamicSettings.sidebarPalette.iconStroke = GColorBlack;
  }

  // dark regions draw directly on the watchface background
  dynamicSettings.darkSidebarPalette.background = settings.timeBgColor;
  dynamicSettings.darkSidebarPalette.text = settings.altSidebarTextColor;
  dynamicSettings.darkSidebarPalette.iconFill = settings.timeBgColor;
  dynamicSettings.darkSidebarPalette.iconStroke = settings.timeColor;
}
