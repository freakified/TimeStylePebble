#pragma once
#include <pebble.h>

// language ids index resources/data/languages.json; only the ones the
// code special-cases are named here
#define LANGUAGE_EN 0
#define LANGUAGE_TR 6
#define LANGUAGE_COUNT 37

// not a language: follow the watch's own language, falling back to
// English when it isn't one of ours
#define LANGUAGE_AUTO 255

// must match tools/gen_languages.py, which packs the LANGUAGES resource
#define LANGUAGE_NAME_SIZE 8
#define LANGUAGE_WEEK_SIZE 12
#define LANGUAGE_LOCALE_SIZE 3

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

// loads a language's strings into Language_strings, resolving
// LANGUAGE_AUTO; cheap when that language is already loaded
void Language_load(uint8_t languageId);

// the id of the language currently in Language_strings, never
// LANGUAGE_AUTO
uint8_t Language_loadedId();
