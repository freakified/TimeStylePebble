#include "settings.h"


void Settings_init() {
  // first, try to load saved settings
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    persist_read_data(SETTINGS_PERSIST_KEY, &globalSettings, sizeof(Settings));
  } else {
    // if no persistent data is available, use the defaults
    globalSettings.timeColor      = GColorOrange;
    globalSettings.timeBgColor    = GColorBlack;
    globalSettings.sidebarColor   = GColorOrange;
    globalSettings.sidebarOnRight = true;
    globalSettings.useMetric      = false;
  }
}

void Settings_deinit() {
  // save weather data to persistent storage
  persist_write_data(SETTINGS_PERSIST_KEY, &globalSettings, sizeof(Settings));
}