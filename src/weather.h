#pragma once
#include <pebble.h>

static GDrawCommandImage *currentWeatherIcon;
static int currentTemp;
static uint32_t currentWeatherIconResourceID;

void Weather_setCondition(int conditionCode, bool isNight);
void Weather_init();
void Weather_deinit();