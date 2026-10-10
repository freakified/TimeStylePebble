#include "settings.h"
#include <pebble.h>

Settings settings;
DynamicSettings dynamicSettings;

void Settings_init() {
  Settings_loadFromStorage();

  // for anyone who already installed timestyle on an emery, reset
  // their large font setting, since the old "large" setting is
  // roughly equivalent to the new default, and the new "large"
  // setting is EVEN LARGER
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  if (!persist_exists(MIGRATION_EMERY_FONT_PERSIST_KEY)) {
    if (settings.useLargeFonts) {
      settings.useLargeFonts = false;
      Settings_saveToStorage();
    }
    persist_write_bool(MIGRATION_EMERY_FONT_PERSIST_KEY, true);
  }
#endif
}

void Settings_deinit() { Settings_saveToStorage(); }

// an unknown widget id would still count as "something in the slot" (a
// bottom bar with nothing drawn in it, say), so clear it instead
static void clear_unknown_widgets() {
  for (int i = 0; i < MAX_REGION_SLOTS; i++) {
    if (!SidebarWidgets_isKnownType(settings.vRegionSlots[i])) {
      settings.vRegionSlots[i] = EMPTY;
    }
  }
  for (int i = 0; i < HREGION_SLOT_MAX; i++) {
    if (!SidebarWidgets_isKnownType(settings.hRegionSlots[i])) {
      settings.hRegionSlots[i] = EMPTY;
    }
  }
}

void Settings_loadFromStorage() {
  // Set defaults
  settings.timeBgColor = GColorBlack;

#ifdef PBL_COLOR
  settings.timeColor = GColorOrange;
  settings.sidebarColor = GColorOrange;
#else
  settings.timeColor = GColorWhite;
  settings.sidebarColor = GColorWhite;
#endif

  settings.layoutId = LAYOUT_SIDEBAR_RIGHT;

  for (int i = 0; i < MAX_REGION_SLOTS; i++) {
    settings.vRegionSlots[i] = EMPTY;
  }
  for (int i = 0; i < HREGION_SLOT_MAX; i++) {
    settings.hRegionSlots[i] = EMPTY;
  }

  // widgets are a packed list: the first renders at the top of the
  // sidebar, the last pins to the bottom. Round splits the same list
  // between its two arcs -- slots 0..2 are the left arc, 3..5 the right
  // -- so its defaults have to straddle the split to put one widget in
  // each arc.
#ifdef PBL_ROUND
  settings.vRegionSlots[0] = WEATHER_CURRENT;
  settings.vRegionSlots[ROUND_ARC_SLOT_CAP] = DATE;
#else
  settings.vRegionSlots[0] = WEATHER_CURRENT;
  settings.vRegionSlots[1] = DATE;
#endif

  // the bottom bar's slots are left empty on purpose: the bar exists only
  // when it holds a widget, so a default one here would turn it on for
  // every fresh install

  settings.enableStatusBar = true;
  settings.statusShowConnection = false;
  strncpy(settings.altclockName, "UTC", sizeof(settings.altclockName));
  settings.altclockOffset = 0;
  strncpy(settings.altclock2Name, "TYO", sizeof(settings.altclock2Name));
  settings.altclock2Offset = 540;
  settings.localUtcOffset = 0;
  settings.decimalSeparator = '.';
  settings.useWideSidebar = false;
  settings.colorBothArcs = false;

  // settings are persisted as a raw struct, so a blob from any other
  // version would load scrambled; until there's a migration path, fall
  // back to the defaults instead
  if (persist_exists(SETTINGS_PERSIST_KEY) &&
      persist_read_int(SETTINGS_VERSION_PERSIST_KEY) == CURRENT_SETTINGS_VERSION) {
    persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
    settings.altclockName[sizeof(settings.altclockName) - 1] = '\0';
    settings.altclock2Name[sizeof(settings.altclock2Name) - 1] = '\0';
    clear_unknown_widgets();
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage() {
  // newly received settings land here before anything renders them
  clear_unknown_widgets();
  Settings_updateDynamicSettings();

  // We're limited to 256 bytes, so make sure that settings fits
  _Static_assert(sizeof(Settings) <= PERSIST_DATA_MAX_LENGTH,
                 "Warning: settings struct is too large!");
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Current settings size %d", sizeof(settings));

  // Write the data
  persist_write_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
  persist_write_int(SETTINGS_VERSION_PERSIST_KEY, CURRENT_SETTINGS_VERSION);
}

/*
 * Color derivation: text and icon colors come from the region's own color.
 * Mirrored by config-page/src/utils/colorDerivation.ts for the preview;
 * keep the two in sync.
 *
 * Luminances are WCAG relative luminance scaled by 10000. Pebble channels
 * only take four values, so the sRGB-to-linear step is a lookup.
 */
static const uint32_t s_linearChannel[4] = {0, 908, 4020, 10000};

// regions at or below this luminance (0.03) are dark: their icons draw as
// outlines rather than white badges
#define DARK_REGION_LUMINANCE 300

// white text wins when its contrast ratio, times this bias (1.3, in
// tenths), is at least black's: the screen renders mid-tones darker than
// sRGB would suggest
#define WHITE_TEXT_BIAS_TENTHS 13

// an accent needs at least 2.5:1 (5/2) against its region, or the icon
// strokes fall back to the text color
#define MIN_ACCENT_CONTRAST_NUM 5
#define MIN_ACCENT_CONTRAST_DEN 2

static uint32_t luminance(GColor color) {
  return (2126 * s_linearChannel[color.r] + 7152 * s_linearChannel[color.g] +
          722 * s_linearChannel[color.b]) / 10000;
}

// a contrast ratio as a fraction: (lighter + 0.05) / (darker + 0.05)
typedef struct {
  uint32_t num;
  uint32_t den;
} Contrast;

static Contrast contrast(GColor a, GColor b) {
  uint32_t la = luminance(a), lb = luminance(b);
  return la > lb ? (Contrast){la + 500, lb + 500}
                 : (Contrast){lb + 500, la + 500};
}

// whether a > b, by cross-multiplying
static bool contrast_greater(Contrast a, Contrast b) {
  return (uint64_t)a.num * b.den > (uint64_t)b.num * a.den;
}

// black or white, whichever reads best on the background (biased to white)
static GColor best_text_color(GColor background) {
  Contrast white = contrast(background, GColorWhite);
  Contrast black = contrast(background, GColorBlack);
  return (uint64_t)white.num * black.den * WHITE_TEXT_BIAS_TENTHS >=
                 (uint64_t)black.num * white.den * 10
             ? GColorWhite
             : GColorBlack;
}

// a region's text and icon colors, from its background and the two colors
// that could accent its icons (ties go to the first)
static void derive_palette(SidebarPalette *palette, GColor background,
                           GColor accent1, GColor accent2) {
  palette->background = background;
  palette->text = best_text_color(background);

  if (luminance(background) > DARK_REGION_LUMINANCE) {
    palette->iconFill = GColorWhite;
    palette->iconStroke = GColorBlack;
    return;
  }

  // on a dark region, icons are outlines in the more striking accent
  Contrast c1 = contrast(accent1, background);
  Contrast c2 = contrast(accent2, background);
  bool useSecond = contrast_greater(c2, c1);
  GColor accent = useSecond ? accent2 : accent1;
  Contrast best = useSecond ? c2 : c1;

  palette->iconFill = background;
  palette->iconStroke =
      (uint64_t)best.num * MIN_ACCENT_CONTRAST_DEN >=
              (uint64_t)best.den * MIN_ACCENT_CONTRAST_NUM
          ? accent
          : palette->text;
}

void Settings_updateDynamicSettings() {
  dynamicSettings.disableWeather = true;
  dynamicSettings.updateScreenEverySecond = false;
  dynamicSettings.enableBeats = false;
  dynamicSettings.enableAltTimeZone = false;
  dynamicSettings.enablePhonePolling = false;

  // scan the widget slots that can actually render
#ifdef PBL_ROUND
  // both arcs draw from the vertical slot list
  int vSlotCount = MAX_REGION_SLOTS;
  int hSlotCount = 0;
#else
  int vSlotCount = VREGION_SLOT_CAP;
  // the bar needs no presence test of its own: it renders whatever these
  // slots hold, and empty slots contribute nothing to the scan either way
  int hSlotCount = HREGION_SLOT_CAP;
#endif

  for (int i = 0; i < vSlotCount + hSlotCount; i++) {
    SidebarWidgetType widget =
        (SidebarWidgetType)(i < vSlotCount
                                ? settings.vRegionSlots[i]
                                : settings.hRegionSlots[i - vSlotCount]);

    // if there are any weather widgets, enable weather checking
    if (widget == WEATHER_CURRENT ||
        widget == WEATHER_FORECAST_TODAY ||
        widget == WEATHER_UV_INDEX ||
        widget == NEXT_SUN_EVENT ||
        widget == WEATHER_PRECIP_CHANCE) {
      dynamicSettings.disableWeather = false;
      dynamicSettings.enablePhonePolling = true;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if (widget == SECONDS) {
      dynamicSettings.updateScreenEverySecond = true;
    }

    // if any widget is "beats", enable the beats calculation
    if (widget == BEATS) {
      dynamicSettings.enableBeats = true;
    }

    // world time widgets need the shifted-clock calculation, plus periodic
    // phone contact so the city's UTC offset tracks DST changes
    if (widget == WORLD_TIME_1 || widget == WORLD_TIME_2) {
      dynamicSettings.enableAltTimeZone = true;
      dynamicSettings.enablePhonePolling = true;
    }
  }

  // the sidebar wears the sidebar color; its icons can be accented with
  // the time or watchface background color
  derive_palette(&dynamicSettings.sidebarPalette, settings.sidebarColor,
                 settings.timeColor, settings.timeBgColor);

  // dark regions (the bottom bar) draw directly on the watchface
  // background; their icons can be accented with the time or sidebar color
  derive_palette(&dynamicSettings.darkSidebarPalette, settings.timeBgColor,
                 settings.timeColor, settings.sidebarColor);
}
