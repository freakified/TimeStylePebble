#pragma once
#include <pebble.h>

// #define KEY_LOCATION_LAT           0
// #define KEY_LOCATION_LNG           1
// #define KEY_GMT_OFFSET             2
#define KEY_TEMPERATURE                 3
#define KEY_CONDITION_CODE              4
#define KEY_USE_NIGHT_ICON              5
#define KEY_SETTING_COLOR_TIME          6
#define KEY_SETTING_COLOR_BG            7
#define KEY_SETTING_COLOR_SIDEBAR       8
#define KEY_SETTING_SIDEBAR_RIGHT       9
#define KEY_SETTING_USE_METRIC          10
#define KEY_SETTING_BT_VIBE             11
#define KEY_SETTING_SIDEBAR_TEXT_COLOR  12
#define KEY_SETTING_LANGUAGE_ID         13
#define KEY_SETTING_SHOW_BATTERY_METER  14
#define KEY_SETTING_SHOW_LEADING_ZERO   15
#define KEY_SETTING_SHOW_BATTERY_PCT    16
#define KEY_SETTING_DISABLE_WEATHER     17
#define KEY_SETTING_CLOCK_FONT_ID       18
#define KEY_SETTING_HOURLY_VIBE         19
#define KEY_SETTING_ONLY_SHOW_BATTERY_WHEN_LOW 20
#define KEY_SETTING_USE_LARGE_FONTS     21

void messaging_requestNewWeatherData();

void (*message_processed_callback)(void);

extern void messaging_init(void (*message_processed_callback)(void));
extern void inbox_received_callback(DictionaryIterator *iterator, void *context);
extern void inbox_dropped_callback(AppMessageResult reason, void *context);
extern void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
extern void outbox_sent_callback(DictionaryIterator *iterator, void *context);
