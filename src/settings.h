#pragma once
#include <pebble.h>
  
#define SETTINGS_PERSIST_KEY 3

typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  bool useMetric;
  bool sidebarOnRight;
} Settings;

Settings globalSettings;

void Settings_init();
void Settings_deinit();