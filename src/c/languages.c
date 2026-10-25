#include <pebble.h>
#include "languages.h"

_Static_assert(sizeof(LanguageStrings) ==
                   19 * LANGUAGE_NAME_SIZE + LANGUAGE_WEEK_SIZE,
               "LanguageStrings must match the resource's record layout");

LanguageStrings Language_strings;

static int loadedLanguageId = -1;

void Language_load(uint8_t languageId) {
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
