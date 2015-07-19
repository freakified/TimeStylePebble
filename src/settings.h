#pragma once
#include <pebble.h>

#define SETTINGS_PERSIST_KEY 3
#define SETTINGS_VERSION_KEY 4

// settings for app version 1.2
#define CURRENT_SETTINGS_VERSION 2

// language ids
#define LANGUAGE_EN 0
#define LANGUAGE_FR 1
#define LANGUAGE_DE 2
#define LANGUAGE_ES 3
#define LANGUAGE_IT 4

typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  bool useMetric;
  bool sidebarOnRight;
} Settings_v0;

typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;
  bool useMetric;
  bool sidebarOnRight;
  bool btVibe;
  bool showBatteryLevel; // not yet used
  uint8_t languageId;
} Settings;

Settings globalSettings;

void Settings_init();
void Settings_loadAllDefaults();
void Settings_loadV2Defaults();
void Settings_deinit();
