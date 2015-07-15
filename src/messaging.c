#include "weather.h"
#include "messaging.h"
#include "settings.h"
  
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
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Watch messaging is started!");
  app_message_register_inbox_received(inbox_received_callback);
}

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // does this message contain weather information?
  Tuple *weatherTemp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *weatherConditions_tuple = dict_find(iterator, KEY_CONDITION_CODE);
  Tuple *weatherIsNight_tuple = dict_find(iterator, KEY_USE_NIGHT_ICON);
  
  if(weatherTemp_tuple != NULL && weatherConditions_tuple != NULL && weatherIsNight_tuple != NULL) {
    bool isNight = (bool)weatherIsNight_tuple->value->int32;
    
    // now set the weather conditions properly
    Weather_weatherInfo.currentTemp = (int)weatherTemp_tuple->value->int32;
    Weather_setCondition((int)weatherConditions_tuple->value->int32, isNight);
  }
  
  // does this message contain new config information?
  Tuple *timeColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_TIME);
  Tuple *bgColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_BG);
  Tuple *sidebarColor_tuple = dict_find(iterator, KEY_SETTING_COLOR_SIDEBAR);
  Tuple *sidebarPos_tuple = dict_find(iterator, KEY_SETTING_SIDEBAR_RIGHT);
  Tuple *useMetric_tuple = dict_find(iterator, KEY_SETTING_USE_METRIC);
  
  if(timeColor_tuple != NULL) {
    globalSettings.timeColor = GColorFromHEX(timeColor_tuple->value->int32);
  }
  
  if(bgColor_tuple != NULL) {
    globalSettings.timeBgColor = GColorFromHEX(bgColor_tuple->value->int32);
  }
  
  if(sidebarColor_tuple != NULL) {
    globalSettings.sidebarColor = GColorFromHEX(sidebarColor_tuple->value->int32);
  }
  
  if(sidebarPos_tuple != NULL) {
    globalSettings.sidebarOnRight = (bool)sidebarPos_tuple->value->int8;
  }
  
  if(useMetric_tuple != NULL) {
    globalSettings.useMetric = (bool)useMetric_tuple->value->int8;
  }
  
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