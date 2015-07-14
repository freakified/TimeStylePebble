#include "location_info.h"
#include "bgpicker.h"
#include "weather.h"
#include "messaging.h"
  
void messaging_requestNewWeatherData() {
  // attempt to send the stored location
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  // TODO: make stored location actually work
  dict_write_uint32(iter, KEY_LOCATION_LAT, (int)(bgpicker_location.lat * 1000000));
  dict_write_uint32(iter, KEY_LOCATION_LNG, (int)(bgpicker_location.lng * 1000000));
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
  Tuple *lat_tuple = dict_find(iterator, KEY_LOCATION_LAT);
  Tuple *lng_tuple = dict_find(iterator, KEY_LOCATION_LNG);
  Tuple *tzOffset_tuple = dict_find(iterator, KEY_GMT_OFFSET);
  Tuple *weatherTemp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *weatherConditions_tuple = dict_find(iterator, KEY_CONDITION_CODE);
  Tuple *weatherIsNight_tuple = dict_find(iterator, KEY_USE_NIGHT_ICON);
  
  if (lat_tuple != NULL && lng_tuple != NULL && tzOffset_tuple != NULL) {
    LocationInfo loc; 
    
    // set our location thing
    float lat = (int)lat_tuple->value->int32;
    lat /= 1000000;
    
    float lng = (int)lng_tuple->value->int32;
    lng /= 1000000;
    
    float tzOffset = (int)tzOffset_tuple->value->int32;
    tzOffset /= 60;
    
    loc.lat = lat;
    loc.lng = lng;
    loc.tzOffset = tzOffset;
    
    bgpicker_updateLocation(loc);
    
  }
  
  if(weatherTemp_tuple != NULL && weatherConditions_tuple != NULL && weatherIsNight_tuple != NULL) {
    bool isNight = (bool)weatherIsNight_tuple->value->int32;
    printf("The night value is: %d", (int)weatherIsNight_tuple->value->int32);
    
    // now set the weather conditions properly
    Weather_weatherInfo.currentTemp = (int)weatherTemp_tuple->value->int32;
    Weather_setCondition((int)weatherConditions_tuple->value->int32, isNight);
    
    // essentially, this forces the sidebar in main to redraw
    message_processed_callback();
  }
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