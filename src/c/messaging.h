#pragma once
#include <pebble.h>

void messaging_requestNewWeatherData();
void messaging_sendConfigSnapshot();

void messaging_init(void (*message_processed_callback)(void));
