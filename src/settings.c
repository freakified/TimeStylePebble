#include "settings.h"

void Settings_init() {
  // first, check if we have any saved settings
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    // if we do, what version of the settings was saved?
    uint32_t savedVersion = persist_read_int(SETTINGS_VERSION_KEY);

    if(savedVersion == 2) { // in this case, we can direclty load the current version
      persist_read_data(SETTINGS_PERSIST_KEY, &globalSettings, sizeof(Settings));
      persist_delete(SETTINGS_PERSIST_KEY);
    } else { // in this case, we're upgrading from the old version

        // first, read in the settings using the original format
        Settings_v0 oldSettings;
        persist_read_data(SETTINGS_PERSIST_KEY, &oldSettings, sizeof(Settings_v0));
        persist_delete(SETTINGS_PERSIST_KEY);

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

  // load the individual settings added in newer versions
  // if they don't exist, we get "0", which is the default anyway
  Settings_showLeadingZero = persist_read_int(SETTING_LEADING_ZERO_KEY);
  Settings_showBatteryPct = persist_read_bool(SETTING_SHOW_BATTERY_PCT_KEY);
  Settings_disableWeather = persist_read_bool(SETTING_DISABLE_WEATHER_KEY);
  Settings_clockFontId = persist_read_bool(SETTING_CLOCK_FONT_ID_KEY);
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
  persist_write_bool(SETTING_SHOW_BATTERY_PCT_KEY, Settings_showBatteryPct);
    
    // switch(status) {
    //   case S_SUCCESS:
    //     printf("S_SUCCESS");
    //     break;
    //   case E_ERROR:
    //     printf("E_ERROR");
    //     break;
    //   case E_UNKNOWN:
    //     printf("E_UNKNOWN");
    //     break;
    //   case E_INTERNAL:
    //     printf("E_INTERNAL");
    //     break;
    //   case E_INVALID_ARGUMENT:
    //     printf("E_INVALID_ARGUMENT");
    //     break;
    //   case E_OUT_OF_MEMORY:
    //     printf("E_OUT_OF_MEMORY");
    //     break;
    //   case E_OUT_OF_STORAGE:
    //     printf("E_OUT_OF_STORAGE");
    //     break;
    //   case E_OUT_OF_RESOURCES:
    //     printf("E_OUT_OF_RESOURCES");
    //     break;
    //   case E_RANGE:
    //     printf("E_RANGE");
    //     break;
    //   case E_DOES_NOT_EXIST:
    //     printf("E_DOES_NOT_EXIST");
    //     break;
    //   case E_INVALID_OPERATION:
    //     printf("E_INVALID_OPERATION");
    //     break;
    //   case E_BUSY:
    //     printf("E_BUSY");
    //     break;
    //   case S_TRUE:
    //     printf("S_TRUE");
    //     break;
    //   case S_NO_MORE_ITEMS:
    //     printf("S_NO_MORE_ITEMS");
    //     break;
    //   case S_NO_ACTION_REQUIRED:
    //     printf("S_NO_ACTION_REQUIRED");
    //     break;
    //   default:
    //     printf("Something else...");
    //     break;
    // }

  persist_write_bool(SETTING_DISABLE_WEATHER_KEY, Settings_disableWeather);
  persist_write_bool(SETTING_CLOCK_FONT_ID_KEY, Settings_clockFontId);
  persist_write_int(SETTINGS_VERSION_KEY, CURRENT_SETTINGS_VERSION);
}
