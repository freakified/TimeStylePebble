#include <pebble.h>
#include "weather.h"
#include "settings.h"
#include "messaging.h"

void (*message_processed_callback)(void);

void messaging_requestNewWeatherData() {
  // just send an empty message for now
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint32(iter, 0, 0);
  app_message_outbox_send();
}

void messaging_init(void (*processed_callback)(void)) {
  // register my custom callback
  message_processed_callback = processed_callback;

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(1024, 8);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Watch messaging is started!");
  app_message_register_inbox_received(inbox_received_callback);
}

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // does this message contain weather information?
  Tuple *weatherTemp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *weatherConditions_tuple = dict_find(iterator, KEY_CONDITION_CODE);
  Tuple *weatherIsNight_tuple = dict_find(iterator, KEY_USE_NIGHT_ICON);

  // forecast info
  Tuple *weatherForecastCondition_tuple = dict_find(iterator, KEY_FORECAST_CONDITION);
  Tuple *weatherForecastHigh_tuple = dict_find(iterator, KEY_FORECAST_TEMP_HIGH);
  Tuple *weatherForecastLow_tuple = dict_find(iterator, KEY_FORECAST_TEMP_LOW);

  if(weatherTemp_tuple != NULL && weatherConditions_tuple != NULL && weatherIsNight_tuple != NULL) {
    bool isNight = (bool)weatherIsNight_tuple->value->int32;

    // now set the weather conditions properly
    Weather_weatherInfo.currentTemp = (int)weatherTemp_tuple->value->int32;
    Weather_weatherForecast.highTemp = (int)weatherForecastHigh_tuple->value->int32;
    Weather_weatherForecast.lowTemp = (int)weatherForecastLow_tuple->value->int32;

    Weather_setConditions(weatherConditions_tuple->value->int32, isNight,
                          weatherForecastCondition_tuple->value->int32);

    Weather_saveData();
  }

  // does this message contain new config information?
  Tuple *timeColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_TIME);
  Tuple *bgColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_BG);
  Tuple *sidebarColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_SIDEBAR);
  Tuple *sidebarPos_tuple = dict_find(iterator, KEY_SETTING_SIDEBAR_LEFT);
  Tuple *sidebarTextColor_tuple = dict_find(iterator, KEY_SETTING_SIDEBAR_TEXT_COLOR);
  Tuple *useMetric_tuple = dict_find(iterator, KEY_SETTING_USE_METRIC);
  Tuple *btVibe_tuple = dict_find(iterator, KEY_SETTING_BT_VIBE);
  Tuple *language_tuple = dict_find(iterator, KEY_SETTING_LANGUAGE_ID);
  Tuple *leadingZero_tuple = dict_find(iterator, KEY_SETTING_SHOW_LEADING_ZERO);
  Tuple *batteryPct_tuple = dict_find(iterator, KEY_SETTING_SHOW_BATTERY_PCT);
  Tuple *disableWeather_tuple = dict_find(iterator, KEY_SETTING_DISABLE_WEATHER);
  Tuple *clockFont_tuple = dict_find(iterator, KEY_SETTING_CLOCK_FONT_ID);
  Tuple *hourlyVibe_tuple = dict_find(iterator, KEY_SETTING_HOURLY_VIBE);
  Tuple *onlyShowBatteryWhenLow_tuple = dict_find(iterator, KEY_SETTING_ONLY_SHOW_BATTERY_WHEN_LOW);
  Tuple *useLargeFonts_tuple = dict_find(iterator, KEY_SETTING_USE_LARGE_FONTS);

  Tuple *widget0Id_tuple = dict_find(iterator, KEY_WIDGET_0_ID);
  Tuple *widget1Id_tuple = dict_find(iterator, KEY_WIDGET_1_ID);
  Tuple *widget2Id_tuple = dict_find(iterator, KEY_WIDGET_2_ID);

  Tuple *altclockName_tuple = dict_find(iterator, KEY_SETTING_ALTCLOCK_NAME);
  Tuple *altclockOffset_tuple = dict_find(iterator, KEY_SETTING_ALTCLOCK_OFFSET);

  if(timeColor_tuple != NULL) {
    globalSettings.timeColor = GColorFromHEX(timeColor_tuple->value->int32);
  }

  if(bgColor_tuple != NULL) {
    globalSettings.timeBgColor = GColorFromHEX(bgColor_tuple->value->int32);
  }

  if(sidebarColor_tuple != NULL) {
    globalSettings.sidebarColor = GColorFromHEX(sidebarColor_tuple->value->int32);
  }

  if(sidebarTextColor_tuple != NULL) {
    // text can only be black or white, so we'll enforce that here
    globalSettings.sidebarTextColor = GColorFromHEX(sidebarTextColor_tuple->value->int32);
  }

  if(sidebarPos_tuple != NULL) {
    globalSettings.sidebarOnLeft = (bool)sidebarPos_tuple->value->int8;
  }

  if(useMetric_tuple != NULL) {
    globalSettings.useMetric = (bool)useMetric_tuple->value->int8;
  }

  if(btVibe_tuple != NULL) {
    globalSettings.btVibe = (bool)btVibe_tuple->value->int8;
  }

  if(leadingZero_tuple != NULL) {
    globalSettings.showLeadingZero = (bool)leadingZero_tuple->value->int8;
  }

  if(batteryPct_tuple != NULL) {
    globalSettings.showBatteryPct = (bool)batteryPct_tuple->value->int8;
  }

  if(onlyShowBatteryWhenLow_tuple != NULL) {
    globalSettings.onlyShowBatteryWhenLow = (bool)onlyShowBatteryWhenLow_tuple->value->int8;
  }

  if(disableWeather_tuple != NULL) {
    globalSettings.disableWeather = (bool)disableWeather_tuple->value->int8;
  }

  if(clockFont_tuple != NULL) {
    globalSettings.clockFontId = clockFont_tuple->value->int8;
  }

  if(useLargeFonts_tuple != NULL) {
    globalSettings.useLargeFonts = (bool)useLargeFonts_tuple->value->int8;
  }

  if(hourlyVibe_tuple != NULL) {
    globalSettings.hourlyVibe = hourlyVibe_tuple->value->int8;
  }

  if(language_tuple != NULL) {
    globalSettings.languageId = language_tuple->value->int8;
  }

  if(language_tuple != NULL) {
    globalSettings.languageId = language_tuple->value->int8;
  }

  if(widget0Id_tuple != NULL) {
    globalSettings.widgets[0] = widget0Id_tuple->value->int8;
  }

  if(widget1Id_tuple != NULL) {
    globalSettings.widgets[1] = widget1Id_tuple->value->int8;
  }

  if(widget2Id_tuple != NULL) {
    globalSettings.widgets[2] = widget2Id_tuple->value->int8;
  }

  if(altclockName_tuple != NULL) {
    strncpy(globalSettings.altclockName, altclockName_tuple->value->cstring, sizeof(globalSettings.altclockName));
  }

  if(altclockOffset_tuple != NULL) {
    globalSettings.altclockOffset = altclockOffset_tuple->value->int8;
  }

  // save the new settings to persistent storage
  Settings_saveToStorage();

  // notify the main screen, in case something changed
  message_processed_callback();
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d %d %d", reason, APP_MSG_SEND_TIMEOUT, APP_MSG_SEND_REJECTED);

}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
