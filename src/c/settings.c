#include "settings.h"
#include <pebble.h>
#include "languages.h"

Settings settings;
DynamicSettings dynamicSettings;

void Settings_init() {
  Settings_loadFromStorage();
}

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

// master encoded the clock digits' weight in clockFontId: Avenir's "bold"
// ids, and LECO, which only had its regular (now "bold") weight
static void migrate_clock_font() {
  settings.hoursBold = false;
  settings.minutesBold = false;

  switch (settings.clockFontId) {
    case FONT_SETTING_LEGACY_BOLD:
      settings.hoursBold = settings.minutesBold = true;
      break;
    case FONT_SETTING_LEGACY_BOLD_H:
      settings.hoursBold = true;
      break;
    case FONT_SETTING_LEGACY_BOLD_M:
      settings.minutesBold = true;
      break;
    case FONT_SETTING_LECO:
      settings.hoursBold = settings.minutesBold = true;
      return;
    default:
      return;
  }
  settings.clockFontId = FONT_SETTING_AVENIR;
}

/*
 * Migration from master (7.x). Master persisted this struct raw, so it's
 * frozen here exactly as master declared it. arm-none-eabi packs enums
 * into a byte, hence the uint8_t fields standing in for master's enums.
 */
typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;
  uint8_t languageId;
  bool showLeadingZero;
  uint8_t clockFontId;
  bool btVibe;
  uint8_t hourlyVibe;
  uint8_t widgets[3];
  bool sidebarOnLeft;
  bool useLargeFonts;
  bool activateDisconnectIcon;
  bool useMetric;
  bool showBatteryPct;
  bool disableAutobattery;
  char altclockName[8];
  int altclockOffset; // whole hours from UTC
  bool healthUseDistance;
  bool healthUseRestfulSleep;
  char decimalSeparator;
} MasterSettings;

_Static_assert(sizeof(MasterSettings) == 36,
               "MasterSettings must match master's persisted layout");

// master's bluetooth disconnection widget, since replaced by the status bar
#define MASTER_WIDGET_BLUETOOTH_DISCONNECT 1

// a flag master's emery font migration left behind
#define MASTER_EMERY_FONT_PERSIST_KEY 200

static uint8_t migrate_widget(uint8_t widget, bool healthUseDistance) {
  if (widget == MASTER_WIDGET_BLUETOOTH_DISCONNECT) {
    return EMPTY;
  }

  // master's step counter could show distance instead, which is now its
  // own widget
  if (widget == STEP_COUNTER && healthUseDistance) {
    return DISTANCE_WALKED;
  }

  return widget;
}

// translates master's persisted settings onto the defaults already in
// `settings`. Settings master didn't have keep their defaults; master's
// sidebar text color and auto-battery setting have no equivalent
static void migrate_from_master() {
  MasterSettings old;
  persist_read_data(SETTINGS_PERSIST_KEY, &old, sizeof(old));

  settings.timeColor = old.timeColor;
  settings.timeBgColor = old.timeBgColor;
  settings.sidebarColor = old.sidebarColor;

  settings.languageId = old.languageId;
  settings.showLeadingZero = old.showLeadingZero;
  settings.clockFontId = old.clockFontId;
  migrate_clock_font();

  settings.btVibe = old.btVibe;
  settings.hourlyVibe = (VibeIntervalType)old.hourlyVibe;

  settings.layoutId =
      old.sidebarOnLeft ? LAYOUT_SIDEBAR_LEFT : LAYOUT_SIDEBAR_RIGHT;
  settings.useLargeFonts = old.useLargeFonts;
  settings.useMetric = old.useMetric;
  settings.healthUseRestfulSleep = old.healthUseRestfulSleep;
  settings.decimalSeparator = old.decimalSeparator;

  // the world time keeps master's fixed offset until the user picks a
  // city; the phone supplies localUtcOffset, which the shifted clock
  // also needs
  memcpy(settings.altclockName, old.altclockName, sizeof(old.altclockName));
  settings.altclockOffset = old.altclockOffset * 60;

  // master's widgets were top / middle / bottom, with empty slots holding
  // their place; the new list is packed, its last widget pinned to the
  // bottom. Master's round face drew only the first (left arc) and the
  // last (right arc)
  for (int i = 0; i < MAX_REGION_SLOTS; i++) {
    settings.vRegionSlots[i] = EMPTY;
  }
#ifdef PBL_ROUND
  settings.vRegionSlots[0] =
      migrate_widget(old.widgets[0], old.healthUseDistance);
  settings.vRegionSlots[ROUND_ARC_SLOT_CAP] =
      migrate_widget(old.widgets[2], old.healthUseDistance);
#else
  int slot = 0;
  for (int i = 0; i < 3; i++) {
    uint8_t widget = migrate_widget(old.widgets[i], old.healthUseDistance);
    if (widget != EMPTY) {
      settings.vRegionSlots[slot++] = widget;
    }
  }
#endif

  // the status bar took over the disconnection icon, and the widget
  bool hadDisconnectWidget = false;
  for (int i = 0; i < 3; i++) {
    if (old.widgets[i] == MASTER_WIDGET_BLUETOOTH_DISCONNECT) {
      hadDisconnectWidget = true;
    }
  }
  settings.enableStatusBar = old.activateDisconnectIcon || hadDisconnectWidget;

  // the battery percentage is now one of the widget's hideable parts
  if (!old.showBatteryPct) {
    settings.hideWidgetDetail |= 1u << BATTERY_METER;
  }

  persist_delete(MASTER_EMERY_FONT_PERSIST_KEY);
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
  strncpy(settings.altclock2Name, "UTC", sizeof(settings.altclock2Name));
  settings.altclock2Offset = 0;
  settings.localUtcOffset = 0;
  settings.decimalSeparator = '.';
  settings.languageId = LANGUAGE_AUTO;
  settings.useWideSidebar = false;
  settings.colorBothArcs = false;
  settings.clockFontId = FONT_SETTING_AVENIR;
  settings.hoursBold = false;
  settings.minutesBold = false;
  settings.hideWidgetMarker = 0;
  settings.hideWidgetDetail = 0;

  // settings are persisted as a raw struct, so a blob from any other
  // version would load scrambled: master's is migrated, and anything else
  // (an unreleased development version) falls back to the defaults
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    int storedVersion = persist_read_int(SETTINGS_VERSION_PERSIST_KEY);
    if (storedVersion == CURRENT_SETTINGS_VERSION) {
      persist_read_data(SETTINGS_PERSIST_KEY, &settings, sizeof(settings));
    } else if (storedVersion == SETTINGS_VERSION_MASTER) {
      migrate_from_master();
    }
    settings.altclockName[sizeof(settings.altclockName) - 1] = '\0';
    settings.altclock2Name[sizeof(settings.altclock2Name) - 1] = '\0';
    clear_unknown_widgets();

    if (storedVersion == SETTINGS_VERSION_MASTER) {
      Settings_saveToStorage();
    }
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

// regions at or below this luminance (0.1) are dark: their icons draw as
// outlines rather than white badges
#define DARK_REGION_LUMINANCE 1000

// white text wins when its contrast ratio, times this bias (1.4, in
// tenths), is at least black's: the screen renders mid-tones darker than
// sRGB would suggest
#define WHITE_TEXT_BIAS_TENTHS 14

// an accent needs at least 4:1 (4/1) against its region, or the icon
// strokes fall back to the text color
#define MIN_ACCENT_CONTRAST_NUM 4
#define MIN_ACCENT_CONTRAST_DEN 1

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
  dynamicSettings.enableHeartRate = false;
  dynamicSettings.enablePhonePolling = false;

  // scan the widget slots that can actually render
  // on round both arcs draw from the vertical slot list. The bar needs
  // no presence test of its own: it renders whatever these slots hold,
  // and empty slots contribute nothing to the scan either way
  int vSlotCount = VREGION_SLOT_CAP;
  int hSlotCount = HREGION_SLOT_CAP;

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

    // heart rate readings arrive asynchronously, and only the heart rate
    // widget needs a redraw for them
    if (widget == HEARTRATE) {
      dynamicSettings.enableHeartRate = true;
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
