#include "settings.h"

void Settings_init() {
  // first, check if we have any saved settings
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    // if we do, what version of the settings was saved?
    uint32_t savedVersion = persist_read_int(SETTINGS_VERSION_KEY);

    if(savedVersion == 2) { // in this case, we can direclty load the current version
      persist_read_data(SETTINGS_PERSIST_KEY, &globalSettings, sizeof(Settings));
    } else { // in this case, we're upgrading from the old version

        // first, read in the settings using the original format
        Settings_v0 oldSettings;
        persist_read_data(SETTINGS_PERSIST_KEY, &oldSettings, sizeof(Settings_v0));

        // now upgrade to the new format
        globalSettings.timeColor = oldSettings.timeColor;
        globalSettings.timeBgColor = oldSettings.timeBgColor;
        globalSettings.sidebarColor = oldSettings.sidebarColor;
        globalSettings.useMetric = oldSettings.useMetric;
        globalSettings.sidebarOnRight = oldSettings.sidebarOnRight;

        // fill the new settings, use the defaults
        Settings_loadV2Defaults();
    }
  } else {
    // if there weren't any saved settings, load the defaults
    Settings_loadAllDefaults();
  }

  // if it doesn't exist, we get "0", which is the default anyway
  Settings_showLeadingZero = persist_read_int(SETTING_LEADING_ZERO_KEY);
}

void Settings_loadAllDefaults() {
  // if no persistent data is available, use the defaults

  // v1 settings
  #ifdef PBL_COLOR
    globalSettings.timeColor      = GColorOrange;
    globalSettings.sidebarColor   = GColorOrange;
  #else
    globalSettings.timeColor      = GColorWhite;
    globalSettings.sidebarColor   = GColorWhite;
  #endif
  globalSettings.timeBgColor    = GColorBlack;
  globalSettings.sidebarOnRight = true;
  globalSettings.useMetric      = false;

  Settings_loadV2Defaults();
}

void Settings_loadV2Defaults() {
  // load the v2-added defaults

  // settings added in v2
  globalSettings.sidebarTextColor   = GColorBlack;
  globalSettings.btVibe             = false;
  globalSettings.languageId         = LANGUAGE_EN;
  globalSettings.showBatteryLevel   = false;
}

void Settings_deinit() {
  // save settings to persistent storage
  persist_write_data(SETTINGS_PERSIST_KEY, &globalSettings, sizeof(Settings));
  persist_write_int(SETTING_LEADING_ZERO_KEY, Settings_showLeadingZero);
  persist_write_int(SETTINGS_VERSION_KEY, CURRENT_SETTINGS_VERSION);
}
