#include <pebble.h>
#include "languages.h"

_Static_assert(sizeof(LanguageStrings) ==
                   19 * LANGUAGE_NAME_SIZE + LANGUAGE_WEEK_SIZE,
               "LanguageStrings must match the resource's record layout");

LanguageStrings Language_strings;

static int loadedLanguageId = -1;

// the watch locale the automatic language was last resolved for, so the
// locale table is only searched when it changes
static char resolvedLocale[2];
static uint8_t resolvedLanguageId = LANGUAGE_EN;

// matches the watch locale's language ("de" of "de_DE") against the
// locale table that follows the strings in the LANGUAGES resource
static uint8_t resolve_auto() {
  const char *locale = i18n_get_system_locale();
  if (locale == NULL || locale[0] == '\0' || locale[1] == '\0') {
    return LANGUAGE_EN;
  }

  if (locale[0] == resolvedLocale[0] && locale[1] == resolvedLocale[1]) {
    return resolvedLanguageId;
  }

  ResHandle handle = resource_get_handle(RESOURCE_ID_LANGUAGES);
  size_t tableStart = LANGUAGE_COUNT * sizeof(LanguageStrings);
  size_t tableSize = resource_size(handle) - tableStart;
  uint8_t entry[LANGUAGE_LOCALE_SIZE];

  resolvedLanguageId = LANGUAGE_EN;
  for (size_t offset = 0; offset + LANGUAGE_LOCALE_SIZE <= tableSize;
       offset += LANGUAGE_LOCALE_SIZE) {
    resource_load_byte_range(handle, tableStart + offset, entry,
                             LANGUAGE_LOCALE_SIZE);
    if (entry[0] == locale[0] && entry[1] == locale[1]) {
      resolvedLanguageId = entry[2];
      break;
    }
  }
  resolvedLocale[0] = locale[0];
  resolvedLocale[1] = locale[1];

  return resolvedLanguageId;
}

void Language_load(uint8_t languageId) {
  if (languageId == LANGUAGE_AUTO) {
    languageId = resolve_auto();
  }
  if (languageId >= LANGUAGE_COUNT) {
    languageId = LANGUAGE_EN;
  }

  if (languageId == loadedLanguageId) {
    return;
  }

  ResHandle handle = resource_get_handle(RESOURCE_ID_LANGUAGES);
  resource_load_byte_range(handle, languageId * sizeof(LanguageStrings),
                           (uint8_t *)&Language_strings,
                           sizeof(LanguageStrings));
  loadedLanguageId = languageId;
}

uint8_t Language_loadedId() {
  return loadedLanguageId < 0 ? LANGUAGE_EN : loadedLanguageId;
}
