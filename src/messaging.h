#pragma once

#define KEY_LOCATION_LAT 0x0
#define KEY_LOCATION_LNG 0x1
#define KEY_GMT_OFFSET   0x2
  
extern void messaging_init();

extern void inbox_received_callback(DictionaryIterator *iterator, void *context);
extern void inbox_dropped_callback(AppMessageResult reason, void *context);
extern void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
extern void outbox_sent_callback(DictionaryIterator *iterator, void *context);
