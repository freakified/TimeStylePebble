#pragma once
#include <pebble.h>

// Message keys
enum {
  MESSAGE_KEY_WeatherTemperature = 0,
  MESSAGE_KEY_WeatherCondition,
  MESSAGE_KEY_WeatherUVIndex,
  MESSAGE_KEY_WeatherForecastCondition,
  MESSAGE_KEY_WeatherForecastHighTemp,
  MESSAGE_KEY_WeatherForecastLowTemp,
  MESSAGE_KEY_SettingColorTime,
  MESSAGE_KEY_SettingColorBG,
  MESSAGE_KEY_SettingColorSidebar,
  MESSAGE_KEY_SettingSidebarOnLeft,
  MESSAGE_KEY_SettingSidebarTextColor,
  MESSAGE_KEY_SettingUseMetric,
  MESSAGE_KEY_SettingBluetoothVibe,
  MESSAGE_KEY_SettingLanguageID,
  MESSAGE_KEY_SettingShowLeadingZero,
  MESSAGE_KEY_SettingShowBatteryPct,
  MESSAGE_KEY_SettingClockFontId,
  MESSAGE_KEY_SettingHourlyVibe,
  MESSAGE_KEY_SettingUseLargeFonts,
  MESSAGE_KEY_SettingWidget0ID,
  MESSAGE_KEY_SettingWidget1ID,
  MESSAGE_KEY_SettingWidget2ID,
  MESSAGE_KEY_SettingAltClockName,
  MESSAGE_KEY_SettingAltClockOffset,
  MESSAGE_KEY_SettingDecimalSep,
  MESSAGE_KEY_SettingHealthUseDistance,
  MESSAGE_KEY_SettingHealthUseRestfulSleep,
  MESSAGE_KEY_SettingDisableAutobattery,
  MESSAGE_KEY_SettingDisconnectIcon,
  MESSAGE_KEY_BatteryPercent,
  MESSAGE_KEY_IsCharging
};

void messaging_requestNewWeatherData();
void messaging_sendBatteryData(uint8_t batteryPercent, bool isCharging);

void messaging_init(void (*message_processed_callback)(void));
void inbox_received_callback(DictionaryIterator *iterator, void *context);
void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);
