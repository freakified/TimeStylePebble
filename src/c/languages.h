#pragma once
#include <pebble.h>

// language ids index resources/data/languages.json; only the ones the
// code special-cases are named here
#define LANGUAGE_EN 0
#define LANGUAGE_TR 6
#define LANGUAGE_COUNT 37

// must match tools/gen_languages.py, which packs the LANGUAGES resource
#define LANGUAGE_NAME_SIZE 8
#define LANGUAGE_WEEK_SIZE 12

// one language's strings, as stored in the LANGUAGES resource. Only the
// selected language's are kept in RAM.
typedef struct {
  char dayNames[7][LANGUAGE_NAME_SIZE];
  char monthNames[12][LANGUAGE_NAME_SIZE];
  // all of these are taken from:
  // http://www.unicode.org/cldr/charts/28/by_type/date_&_time.fields.html#521165cf49647551
  char wordForWeek[LANGUAGE_WEEK_SIZE];
} LanguageStrings;

extern LanguageStrings Language_strings;

// loads a language's strings into Language_strings; cheap when that
// language is already loaded
void Language_load(uint8_t languageId);
