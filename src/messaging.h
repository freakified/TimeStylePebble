#pragma once
#include <pebble.h>

#define KEY_LOCATION_LAT   0x0
#define KEY_LOCATION_LNG   0x1
#define KEY_GMT_OFFSET     0x2
#define KEY_TEMPERATURE    0x3
#define KEY_CONDITION_CODE 0x4
#define KEY_USE_NIGHT_ICON 0x5

void messaging_requestNewWeatherData();
  
void (*message_processed_callback)(void);
  
extern void messaging_init(void (*message_processed_callback)(void));

extern void inbox_received_callback(DictionaryIterator *iterator, void *context);
extern void inbox_dropped_callback(AppMessageResult reason, void *context);
extern void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
extern void outbox_sent_callback(DictionaryIterator *iterator, void *context);
