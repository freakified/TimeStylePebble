#include <pebble.h>
#include <stddef.h>
#include "weather.h"
#include "settings.h"
#include "messaging.h"

static void (*message_processed_callback)(void);

/*
 * Every setting that travels over AppMessage, in both directions: the
 * phone sends any subset of these when the config page saves, and the
 * watch sends all of them back as a snapshot to prefill the config page.
 * MESSAGE_KEY_* are link-time variables rather than constants, hence the
 * pointers.
 */
typedef enum {
  FIELD_BOOL,   // bool, sent as 0/1
  FIELD_UINT8,  // uint8_t, char, or a byte-sized enum
  FIELD_INT,    // int
  FIELD_UINT32, // uint32_t bitmask
  FIELD_COLOR,  // GColor, sent as 0xRRGGBB
  FIELD_NAME    // char[SETTINGS_NAME_LENGTH]
} FieldType;

typedef struct {
  const uint32_t *key;
  uint8_t offset;
  uint8_t type;
} MessageField;

#define SETTINGS_NAME_LENGTH sizeof(((Settings *)0)->altclockName)
_Static_assert(sizeof(((Settings *)0)->altclock2Name) == SETTINGS_NAME_LENGTH,
               "world time names must share a length");
_Static_assert(sizeof(VibeIntervalType) == sizeof(uint8_t),
               "hourlyVibe is read and written as a uint8");

#define SETTING(keyName, member, fieldType) \
  { &MESSAGE_KEY_##keyName, offsetof(Settings, member), fieldType }

static const MessageField s_settingFields[] = {
  SETTING(SettingColorTime, timeColor, FIELD_COLOR),
  SETTING(SettingColorBG, timeBgColor, FIELD_COLOR),
  SETTING(SettingColorSidebar, sidebarColor, FIELD_COLOR),
  SETTING(SettingLanguageID, languageId, FIELD_UINT8),
  SETTING(SettingShowLeadingZero, showLeadingZero, FIELD_BOOL),
  SETTING(SettingClockFontId, clockFontId, FIELD_UINT8),
  SETTING(SettingClockHoursBold, hoursBold, FIELD_BOOL),
  SETTING(SettingClockMinutesBold, minutesBold, FIELD_BOOL),
  SETTING(SettingBluetoothVibe, btVibe, FIELD_BOOL),
  SETTING(SettingHourlyVibe, hourlyVibe, FIELD_UINT8),
  SETTING(SettingWidget0ID, vRegionSlots[0], FIELD_UINT8),
  SETTING(SettingWidget1ID, vRegionSlots[1], FIELD_UINT8),
  SETTING(SettingWidget2ID, vRegionSlots[2], FIELD_UINT8),
  SETTING(SettingWidget3ID, vRegionSlots[3], FIELD_UINT8),
  SETTING(SettingWidget4ID, vRegionSlots[4], FIELD_UINT8),
  SETTING(SettingWidget5ID, vRegionSlots[5], FIELD_UINT8),
  SETTING(SettingBottomBarWidget0ID, hRegionSlots[0], FIELD_UINT8),
  SETTING(SettingBottomBarWidget1ID, hRegionSlots[1], FIELD_UINT8),
  SETTING(SettingBottomBarWidget2ID, hRegionSlots[2], FIELD_UINT8),
  SETTING(SettingLayoutId, layoutId, FIELD_UINT8),
  SETTING(SettingUseLargeFonts, useLargeFonts, FIELD_BOOL),
  SETTING(SettingUseWideSidebar, useWideSidebar, FIELD_BOOL),
  SETTING(SettingColorBothArcs, colorBothArcs, FIELD_BOOL),
  SETTING(SettingUseMetric, useMetric, FIELD_BOOL),
  SETTING(SettingEnableStatusBar, enableStatusBar, FIELD_BOOL),
  SETTING(SettingStatusShowConnection, statusShowConnection, FIELD_BOOL),
  SETTING(SettingAltClockName, altclockName, FIELD_NAME),
  SETTING(SettingAltClockOffset, altclockOffset, FIELD_INT),
  SETTING(SettingAltClock2Name, altclock2Name, FIELD_NAME),
  SETTING(SettingAltClock2Offset, altclock2Offset, FIELD_INT),
  SETTING(SettingLocalUtcOffset, localUtcOffset, FIELD_INT),
  SETTING(SettingHealthUseRestfulSleep, healthUseRestfulSleep, FIELD_BOOL),
  // the phone sends this one as a one-character string; its first byte
  // is the character either way
  SETTING(SettingDecimalSep, decimalSeparator, FIELD_UINT8),
};

// the plain-number weather fields; the two conditions also swap icons, so
// they're handled separately
#define WEATHER(keyName, member) \
  { &MESSAGE_KEY_##keyName, offsetof(WeatherInfo, member), FIELD_INT }

static const MessageField s_weatherFields[] = {
  WEATHER(WeatherTemperature, currentTemp),
  WEATHER(WeatherUVIndex, currentUVIndex),
  WEATHER(WeatherForecastHighTemp, todaysHighTemp),
  WEATHER(WeatherForecastLowTemp, todaysLowTemp),
  WEATHER(WeatherPrecipChance, precipChance),
  WEATHER(WeatherSunriseMinute, sunriseMinute),
  WEATHER(WeatherSunsetMinute, sunsetMinute),
};

static const MessageField *find_field(const MessageField *fields, int count,
                                      uint32_t key) {
  for (int i = 0; i < count; i++) {
    if (*fields[i].key == key) {
      return &fields[i];
    }
  }
  return NULL;
}

// numbers as wide as they were sent: the phone may pack small values into
// fewer than 4 bytes, and reading 4 off those runs past the tuple
static int32_t tuple_int32(const Tuple *t) {
  switch (t->length) {
    case 1: return (t->type == TUPLE_INT) ? t->value->int8 : t->value->uint8;
    case 2: return (t->type == TUPLE_INT) ? t->value->int16 : t->value->uint16;
    default: return t->value->int32;
  }
}

static uint32_t tuple_uint32(const Tuple *t) {
  return (uint32_t)tuple_int32(t);
}

static void read_field(const MessageField *field, uint8_t *base,
                       const Tuple *t) {
  void *dest = base + field->offset;

  switch (field->type) {
    case FIELD_BOOL:
      *(bool *)dest = t->value->uint8 != 0;
      break;
    case FIELD_UINT8:
      *(uint8_t *)dest = t->value->uint8;
      break;
    case FIELD_INT:
      *(int *)dest = (int)tuple_int32(t);
      break;
    case FIELD_UINT32:
      *(uint32_t *)dest = tuple_uint32(t);
      break;
    case FIELD_COLOR:
      *(GColor *)dest = GColorFromHEX(tuple_uint32(t));
      break;
    case FIELD_NAME:
      strncpy(dest, t->value->cstring, SETTINGS_NAME_LENGTH);
      ((char *)dest)[SETTINGS_NAME_LENGTH - 1] = '\0';
      break;
  }
}

static uint32_t color_to_hex(GColor color) {
  return (color.r * 0x55 << 16) | (color.g * 0x55 << 8) | (color.b * 0x55);
}

static void write_field(DictionaryIterator *iter, const MessageField *field,
                        const uint8_t *base) {
  const void *src = base + field->offset;
  uint32_t key = *field->key;

  switch (field->type) {
    case FIELD_BOOL:
      dict_write_uint8(iter, key, *(const bool *)src ? 1 : 0);
      break;
    case FIELD_UINT8:
      dict_write_uint8(iter, key, *(const uint8_t *)src);
      break;
    case FIELD_INT:
      dict_write_int32(iter, key, *(const int *)src);
      break;
    case FIELD_UINT32:
      dict_write_uint32(iter, key, *(const uint32_t *)src);
      break;
    case FIELD_COLOR:
      dict_write_uint32(iter, key, color_to_hex(*(const GColor *)src));
      break;
    case FIELD_NAME:
      dict_write_cstring(iter, key, src);
      break;
  }
}

void messaging_requestNewWeatherData() {
  // the phone treats any message that isn't a config snapshot as a
  // request for fresh data
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK || iter == NULL) {
    return;
  }
  dict_write_uint32(iter, 0, 0);
  app_message_outbox_send();
}

void messaging_sendConfigSnapshot() {
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK || iter == NULL) {
    return;
  }

  for (size_t i = 0; i < ARRAY_LENGTH(s_settingFields); i++) {
    write_field(iter, &s_settingFields[i], (const uint8_t *)&settings);
  }

  app_message_outbox_send();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  if (dict_find(iterator, MESSAGE_KEY_ConfigSnapshotRequest) != NULL) {
    messaging_sendConfigSnapshot();
    return;
  }

  bool weatherUpdated = false;
  bool settingsUpdated = false;

  for (Tuple *t = dict_read_first(iterator); t != NULL;
       t = dict_read_next(iterator)) {
    const MessageField *field;

    if (t->key == MESSAGE_KEY_WeatherCondition) {
      Weather_setCurrentCondition(tuple_int32(t));
      weatherUpdated = true;
    } else if (t->key == MESSAGE_KEY_WeatherForecastCondition) {
      Weather_setForecastCondition(tuple_int32(t));
      weatherUpdated = true;
    } else if ((field = find_field(s_weatherFields,
                                   ARRAY_LENGTH(s_weatherFields), t->key))) {
      read_field(field, (uint8_t *)&Weather_weatherInfo, t);
      weatherUpdated = true;
    } else if ((field = find_field(s_settingFields,
                                   ARRAY_LENGTH(s_settingFields), t->key))) {
      read_field(field, (uint8_t *)&settings, t);
      settingsUpdated = true;
    }
  }

  // only write to flash what actually changed
  if (weatherUpdated) {
    Weather_saveData();
  }
  if (settingsUpdated) {
    Settings_saveToStorage();
  }

  // notify the main screen, in case something changed
  if (weatherUpdated || settingsUpdated) {
    message_processed_callback();
  }
}

void messaging_init(void (*processed_callback)(void)) {
  message_processed_callback = processed_callback;

  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(512, 512);
}
