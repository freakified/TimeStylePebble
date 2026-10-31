#include "sidebar_widgets.h"
#include "languages.h"
#include "settings.h"
#include "text_metrics.h"
#include "util.h"
#include "weather.h"
#include <pebble.h>
#include <string.h>

#define SIDEBAR_ICON_SIZE 25
#define SIDEBAR_ICON_TEXT_GAP 5
#define SIDEBAR_STACKED_VALUE_GAP 5
#define SIDEBAR_BASIC_LABEL_GAP 3
// between an inline widget's icon, text and words
#define SIDEBAR_INLINE_ITEM_GAP 3
#define SIDEBAR_TEXT_OVERFLOW 5
#define SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT 1
#define SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN 2
#define SIDEBAR_TEXT_FIT_ALLOWANCE 1

#define DATE_CALENDAR_DAY_TOP_PADDING 10
#define DATE_CALENDAR_DAY_BOTTOM_PADDING 4
#define DATE_CALENDAR_DAY_TOP_PADDING_LARGE 6
#define DATE_CALENDAR_DAY_BOTTOM_PADDING_LARGE 4
#define DATE_PLAIN_VERTICAL_PADDING 4
#define DATE_PLAIN_BOX_EXTRA_WIDTH 7
// stacked, the box has the column to itself and can afford more room
#define DATE_STACKED_BOX_EXTRA_WIDTH 2

static GDrawCommandImage *dateImage;
static GDrawCommandImage *batteryImage;
static GDrawCommandImage *batteryChargeImage;
static GDrawCommandImage *sunriseImage;
static GDrawCommandImage *sunsetImage;
static GDrawCommandImage *precipImage;

#ifdef PBL_HEALTH
static GDrawCommandImage *sleepImage;
static GDrawCommandImage *stepsImage;
static GDrawCommandImage *distanceImage;
#ifdef PBL_HEALTH
static MeasurementSystem distanceUnits;
#endif
static GDrawCommandImage *caloriesImage;
static GDrawCommandImage *heartImage;
#endif

#ifdef SIDEBAR_HAS_ALARMS
static GDrawCommandImage *alarmImage;
#endif

static GFont smSidebarFont;
static GFont mdSidebarFont;
static GFont lgSidebarFont;
static GFont currentSidebarFont;
static GFont fallbackSidebarFont;

typedef struct {
  int height;
  int offset;
} SidebarTextMetrics;

typedef struct {
  SidebarTextMetrics sm;
  SidebarTextMetrics md;
  SidebarTextMetrics lg;
  SidebarTextMetrics current;
} SidebarFontMetrics;

typedef struct {
  int calendarTop;
  int calendarHeight;
  int calendarWidth;
  int dayNumTop;
  int monthTop;
  int height;
  bool showWeekday;
  bool showMonth;
} DateLayout;

static SidebarFontMetrics fontMetrics;

// the current weekday and month names point into Language_strings
static const char *currentDayName = "";
static char currentDayNum[5];
static const char *currentMonth = "";
static char currentWeekNum[5];
static char currentSecondsNum[5];
static char currentBeats[5];

// current world times, formatted for display
static SidebarTextCandidates worldTime1Text;
static SidebarTextCandidates worldTime2Text;

// minutes since local midnight, for the sunrise/sunset widget
static int currentMinuteOfDay;

static SidebarContent Empty_getContent();
static SidebarMeasure BatteryMeter_measure(SidebarRenderSpec *spec);
static void BatteryMeter_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec);
static SidebarMeasure date_content_measure(SidebarRenderSpec *spec);
static void date_content_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec);
static SidebarContent CurrentWeather_getContent();
static SidebarContent WeatherForecast_getContent();
static SidebarContent WeekNumber_getContent();
static SidebarContent Seconds_getContent();
static SidebarContent WorldTime1_getContent();
static SidebarContent WorldTime2_getContent();
static SidebarContent NextSunEvent_getContent();
static SidebarContent Beats_getContent();
static SidebarContent UVIndex_getContent();
static SidebarContent PrecipChance_getContent();

#ifdef PBL_HEALTH
static SidebarContent StepCounter_getContent();
static SidebarContent DistanceWalked_getContent();
static SidebarContent ActiveCalories_getContent();
static SidebarContent SleepTimer_getContent();
static SidebarContent HeartRate_getContent();
#endif

#ifdef SIDEBAR_HAS_ALARMS
static SidebarContent NextAlarm_getContent();
#endif

static SidebarTextMetrics gothic_metrics(int fontSize) {
  switch (fontSize) {
  case 14:
    return (SidebarTextMetrics){METRIC_GOTHIC_14_HEIGHT,
                                METRIC_GOTHIC_14_OFFSET};
  case 18:
    return (SidebarTextMetrics){METRIC_GOTHIC_18_HEIGHT,
                                METRIC_GOTHIC_18_OFFSET};
  case 24:
    return (SidebarTextMetrics){METRIC_GOTHIC_24_HEIGHT,
                                METRIC_GOTHIC_24_OFFSET};
  case 28:
    return (SidebarTextMetrics){METRIC_GOTHIC_28_HEIGHT,
                                METRIC_GOTHIC_28_OFFSET};
  default:
    return (SidebarTextMetrics){METRIC_GOTHIC_18_HEIGHT,
                                METRIC_GOTHIC_18_OFFSET};
  }
}

static int text_draw_y(SidebarTextMetrics metrics, int visibleTop) {
  return visibleTop - metrics.offset;
}

static int text_rect_height(SidebarTextMetrics metrics) {
  return metrics.offset + metrics.height;
}

static SidebarTextCandidates
text_candidates(const char *full, const char *compact, const char *tiny) {
  SidebarTextCandidates candidates;
  snprintf(candidates.full, sizeof(candidates.full), "%s", full);
  snprintf(candidates.compact, sizeof(candidates.compact), "%s", compact);
  snprintf(candidates.tiny, sizeof(candidates.tiny), "%s", tiny);
  return candidates;
}

static SidebarTextCandidates same_text_candidates(const char *text) {
  return text_candidates(text, text, text);
}

static void append_text(char *dest, size_t destSize, const char *text) {
  size_t used = strlen(dest);

  if (used >= destSize - 1) {
    return;
  }

  strncat(dest, text, destSize - used - 1);
}

static void join_pair_text(char *dest, size_t destSize, const char *primary,
                           const char *secondary) {
  dest[0] = '\0';
  append_text(dest, destSize, primary);
  append_text(dest, destSize, "/");
  append_text(dest, destSize, secondary);
}

static SidebarTextCandidates
pair_text_candidates(SidebarTextCandidates *primary,
                     SidebarTextCandidates *secondary) {
  SidebarTextCandidates candidates;
  join_pair_text(candidates.full, sizeof(candidates.full), primary->full,
                 secondary->full);
  join_pair_text(candidates.compact, sizeof(candidates.compact),
                 primary->compact, secondary->compact);
  join_pair_text(candidates.tiny, sizeof(candidates.tiny), primary->tiny,
                 secondary->tiny);
  return candidates;
}

/*
 * Joins a label and value into single-string candidates so the whole
 * widget degrades as one unit: drop the value's suffix first, then the
 * label, instead of shrinking the value while the label stays full-size.
 */
static SidebarTextCandidates
label_value_candidates(const char *label, SidebarTextCandidates *value) {
  if (label == NULL || label[0] == '\0') {
    return *value;
  }

  SidebarTextCandidates candidates;
  snprintf(candidates.full, sizeof(candidates.full), "%s %s", label,
           value->full);
  snprintf(candidates.compact, sizeof(candidates.compact), "%s %s", label,
           value->compact);
  snprintf(candidates.tiny, sizeof(candidates.tiny), "%s", value->compact);
  return candidates;
}

static SidebarContent empty_content(SidebarContentKind kind) {
  return (SidebarContent){
      .kind = kind,
      .iconKind = SIDEBAR_ICON_NONE,
      .icon = NULL,
      .label = NULL,
      .primary = same_text_candidates(""),
      .secondary = same_text_candidates(""),
      .isLoading = false,
  };
}

void SidebarWidgets_init() {
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
#else
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
#endif

  dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
  batteryImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
  batteryChargeImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  sunriseImage = gdraw_command_image_create_with_resource(RESOURCE_ID_SUNRISE);
  sunsetImage = gdraw_command_image_create_with_resource(RESOURCE_ID_SUNSET);
  precipImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_WEATHER_HEAVY_RAIN);

#ifdef PBL_HEALTH
  sleepImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_SLEEP);
  stepsImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_STEPS);
  distanceImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_DISTANCE);
  caloriesImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_CALORIES);
  heartImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_HEART);
#endif

#ifdef SIDEBAR_HAS_ALARMS
  alarmImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_ALARM_CLOCK);
#endif
}

void SidebarWidgets_deinit() {
  gdraw_command_image_destroy(dateImage);
  gdraw_command_image_destroy(batteryImage);
  gdraw_command_image_destroy(batteryChargeImage);
  gdraw_command_image_destroy(sunriseImage);
  gdraw_command_image_destroy(sunsetImage);
  gdraw_command_image_destroy(precipImage);

#ifdef PBL_HEALTH
  gdraw_command_image_destroy(stepsImage);
  gdraw_command_image_destroy(distanceImage);
  gdraw_command_image_destroy(caloriesImage);
  gdraw_command_image_destroy(sleepImage);
  gdraw_command_image_destroy(heartImage);
#endif

#ifdef SIDEBAR_HAS_ALARMS
  gdraw_command_image_destroy(alarmImage);
#endif
}

void SidebarWidgets_updateFonts() {
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  fontMetrics.sm = gothic_metrics(18);
  fontMetrics.md = gothic_metrics(24);
  fontMetrics.lg = gothic_metrics(28);
#else
  fontMetrics.sm = gothic_metrics(14);
  fontMetrics.md = gothic_metrics(18);
  fontMetrics.lg = gothic_metrics(24);
#endif

  if (settings.useLargeFonts) {
    currentSidebarFont = lgSidebarFont;
    fallbackSidebarFont = mdSidebarFont;
    fontMetrics.current = fontMetrics.lg;
  } else {
    currentSidebarFont = mdSidebarFont;
    fallbackSidebarFont = smSidebarFont;
    fontMetrics.current = fontMetrics.md;
  }
}

static int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

/*
 * Formats a wall-clock time into text candidates, honoring the 12/24h and
 * leading zero settings. The full candidate carries an optional day-name
 * suffix (used by the world time widgets when the remote date differs);
 * narrow regions naturally fall back to the compact/tiny candidates.
 */
static void format_time_candidates(int hour, int min, const char *daySuffix,
                                   SidebarTextCandidates *out) {
  char timeText[12];
  char tinyText[8];

  if (clock_is_24h_style()) {
    if (settings.showLeadingZero) {
      snprintf(timeText, sizeof(timeText), "%02i:%02i", hour, min);
    } else {
      snprintf(timeText, sizeof(timeText), "%i:%02i", hour, min);
    }
    snprintf(tinyText, sizeof(tinyText), "%i", hour);
  } else {
    char am_pm = (hour < 12) ? 'a' : 'p';
    int hour12 = mod(hour, 12);
    if (hour12 == 0) {
      hour12 = 12;
    }

    if (settings.showLeadingZero) {
      snprintf(timeText, sizeof(timeText), "%02i:%02i%c", hour12, min, am_pm);
    } else {
      snprintf(timeText, sizeof(timeText), "%i:%02i%c", hour12, min, am_pm);
    }
    snprintf(tinyText, sizeof(tinyText), "%i%c", hour12, am_pm);
  }

  if (daySuffix != NULL && daySuffix[0] != '\0') {
    snprintf(out->full, sizeof(out->full), "%s %s", timeText, daySuffix);
  } else {
    snprintf(out->full, sizeof(out->full), "%s", timeText);
  }
  snprintf(out->compact, sizeof(out->compact), "%s", timeText);
  snprintf(out->tiny, sizeof(out->tiny), "%s", tinyText);
}

/*
 * Computes the wall-clock time in another time zone. utcOffsetMinutes is
 * the zone's current offset from UTC, and settings.localUtcOffset is the
 * local zone's; the phone recomputes both from DST rules whenever it
 * talks to the watch (tm_gmtoff is avoided on purpose -- its sign
 * convention isn't trustworthy across firmwares/emulators).
 */
static void update_world_time(const struct tm *localTime, int utcOffsetMinutes,
                              SidebarTextCandidates *out) {
  time_t shifted =
      time(NULL) +
      ((time_t)utcOffsetMinutes - settings.localUtcOffset) * 60;
  struct tm altTime = *localtime(&shifted);

  bool differentDate = altTime.tm_yday != localTime->tm_yday ||
                       altTime.tm_year != localTime->tm_year;
  format_time_candidates(
      altTime.tm_hour, altTime.tm_min,
      differentDate ? Language_strings.dayNames[altTime.tm_wday] : NULL,
      out);
}

#ifdef PBL_HEALTH
static void refresh_health();
#endif

void SidebarWidgets_updateTime(struct tm *timeInfo) {
  // a no-op unless the language setting changed
  Language_load(settings.languageId);

  strftime(currentDayNum, 3, "%e", timeInfo);
  if (currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }

  strftime(currentWeekNum, 3, "%V", timeInfo);

  currentDayName = Language_strings.dayNames[timeInfo->tm_wday];
  currentMonth = Language_strings.monthNames[timeInfo->tm_mon];

  strftime(currentSecondsNum, 4, ":%S", timeInfo);

  int minuteOfDay = timeInfo->tm_hour * 60 + timeInfo->tm_min;

  currentMinuteOfDay = minuteOfDay;

#ifdef PBL_HEALTH
  // also runs on every settings change, so a newly added health widget
  // finds its value cached before it first draws
  refresh_health();
#endif

  if (dynamicSettings.enableBeats) {
    snprintf(currentBeats, sizeof(currentBeats), "%i",
             time_get_beats(timeInfo));
  }

  if (dynamicSettings.enableAltTimeZone) {
    // timeInfo may point at libc's static localtime() buffer, which
    // update_world_time clobbers -- work from a copy
    struct tm localTime = *timeInfo;
    update_world_time(&localTime, settings.altclockOffset, &worldTime1Text);
    update_world_time(&localTime, settings.altclock2Offset, &worldTime2Text);
  }
}

SidebarRenderSpec SidebarWidgets_makeRenderSpec(SidebarLayoutMode mode,
                                                int contentWidth) {
  SidebarWidgets_updateFonts();
  return (SidebarRenderSpec){
      .mode = mode,
      .contentWidth = contentWidth,
  };
}

// the palette widgets draw with; the region renderer sets this before
// drawing, the same way it sets colors on the GContext
static SidebarPalette activePalette;

// transient hide-bit overrides set by the layout cascade in sidebar.c;
// these start as copies of the user's settings and may be tightened for
// the current frame. Reset before each measure pass via
// SidebarWidgets_resetEffectiveHideBits().
static uint32_t effectiveHideMarker;
static uint32_t effectiveHideDetail;

void SidebarWidgets_setActivePalette(const SidebarPalette *palette) {
  activePalette = *palette;
}

void SidebarWidgets_resetEffectiveHideBits(void) {
  effectiveHideMarker = settings.hideWidgetMarker;
  effectiveHideDetail = settings.hideWidgetDetail;
}

void SidebarWidgets_setEffectiveHideBit(uint32_t *field,
                                        SidebarWidgetType type) {
  *field |= 1u << type;
}

uint32_t *SidebarWidgets_effectiveHideMarker(void) {
  return &effectiveHideMarker;
}

uint32_t *SidebarWidgets_effectiveHideDetail(void) {
  return &effectiveHideDetail;
}

static int render_width(SidebarRenderSpec *spec) {
  return spec->contentWidth + SIDEBAR_TEXT_OVERFLOW * 2;
}

static int text_fit_width(SidebarRenderSpec *spec, int textWidth) {
  return spec->mode == SIDEBAR_LAYOUT_INLINE ? textWidth
                                                 : spec->contentWidth;
}

static int text_x(GRect frame, int width) {
  return frame.origin.x + (frame.size.w - width) / 2;
}

static int icon_x(GRect frame) {
  return frame.origin.x + (frame.size.w - SIDEBAR_ICON_SIZE + 1) / 2;
}

// how far an inline icon pushes the text beside it, if there is one
static int inline_icon_advance(bool hasIcon) {
  return hasIcon ? SIDEBAR_ICON_SIZE + SIDEBAR_INLINE_ITEM_GAP : 0;
}

// how far a stacked icon pushes the text below it, if there is one
static int stacked_icon_advance(bool hasIcon) {
  return hasIcon ? SIDEBAR_ICON_SIZE + SIDEBAR_ICON_TEXT_GAP : 0;
}

// text width beside an inline icon (iconAdvance is 0 without one);
// floored at 1 because over-budget squeezes can hand out frames narrower
// than the icon itself, and the firmware text renderer must never see a
// non-positive width
static int inline_text_width(GRect frame, int iconAdvance) {
  int width = frame.size.w - iconAdvance;
  return width < 1 ? 1 : width;
}

static bool text_fits(const char *text, GFont font, GRect rect) {
  GRect measureRect = GRect(0, 0, 200, rect.size.h);
  GSize size = graphics_text_layout_get_content_size(
      text, font, measureRect, GTextOverflowModeFill, GTextAlignmentCenter);

  return size.w <= rect.size.w;
}

static int text_content_width(const char *text, GFont font,
                              SidebarTextMetrics metrics) {
  GSize size = graphics_text_layout_get_content_size(
      text, font, GRect(0, 0, 200, text_rect_height(metrics)),
      GTextOverflowModeFill, GTextAlignmentLeft);
  return size.w;
}

static const char *pick_text(SidebarTextCandidates *candidates,
                             GFont primaryFont, GFont fallbackFont,
                             GRect fitRect, GFont *selectedFont) {
  GFont fonts[2] = {primaryFont, fallbackFont};

  for (int fontIndex = 0; fontIndex < 2; fontIndex++) {
    if (text_fits(candidates->full, fonts[fontIndex], fitRect)) {
      *selectedFont = fonts[fontIndex];
      return candidates->full;
    }

    if (text_fits(candidates->compact, fonts[fontIndex], fitRect)) {
      *selectedFont = fonts[fontIndex];
      return candidates->compact;
    }
  }

  *selectedFont = fallbackFont;
  return candidates->tiny;
}

static GRect text_rect_at_visible_top(int x, int visibleTop, int width,
                                      SidebarTextMetrics metrics) {
  return GRect(x, text_draw_y(metrics, visibleTop), width,
               text_rect_height(metrics));
}

static void draw_text_at_visible_top_aligned(
    GContext *ctx, SidebarTextCandidates *candidates, int x, int visibleTop,
    int width, int fitWidth, GFont primaryFont, GFont fallbackFont,
    SidebarTextMetrics primaryMetrics, SidebarTextMetrics fallbackMetrics,
    GTextAlignment alignment) {
  GFont selectedFont = primaryFont;
  const char *text = pick_text(candidates, primaryFont, fallbackFont,
                               GRect(x, visibleTop, fitWidth,
                                     text_rect_height(primaryMetrics)),
                               &selectedFont);
  SidebarTextMetrics selectedMetrics =
      (selectedFont == primaryFont) ? primaryMetrics : fallbackMetrics;
  GRect drawRect =
      text_rect_at_visible_top(x, visibleTop, width, selectedMetrics);

  graphics_context_set_text_color(ctx, activePalette.text);
  graphics_draw_text(ctx, text, selectedFont, drawRect, GTextOverflowModeFill,
                     alignment, NULL);
}

static void draw_text_at_visible_top(GContext *ctx,
                                     SidebarTextCandidates *candidates, int x,
                                     int visibleTop, int width, int fitWidth,
                                     GFont primaryFont, GFont fallbackFont,
                                     SidebarTextMetrics primaryMetrics,
                                     SidebarTextMetrics fallbackMetrics) {
  draw_text_at_visible_top_aligned(ctx, candidates, x, visibleTop, width,
                                   fitWidth, primaryFont, fallbackFont,
                                   primaryMetrics, fallbackMetrics,
                                   GTextAlignmentCenter);
}

static void draw_literal_text_at_visible_top(GContext *ctx, const char *text,
                                             int x, int visibleTop, int width,
                                             GFont font,
                                             SidebarTextMetrics metrics,
                                             GColor color) {
  graphics_context_set_text_color(ctx, color);
  graphics_draw_text(ctx, text, font,
                     text_rect_at_visible_top(x, visibleTop, width, metrics),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static bool has_icon(SidebarContent *content) {
  return content->iconKind == SIDEBAR_ICON_IMAGE;
}

static void draw_icon(GContext *ctx, SidebarContent *content, GRect iconFrame) {
  if (content->iconKind != SIDEBAR_ICON_IMAGE || !content->icon) {
    return;
  }

  gdraw_command_image_recolor_icon(content->icon, activePalette.iconFill,
                              activePalette.iconStroke);
  gdraw_command_image_draw(ctx, content->icon,
                           GPoint(iconFrame.origin.x, iconFrame.origin.y));
}

static void draw_battery_icon(GContext *ctx, GRect iconFrame,
                              uint8_t batteryPercent, bool batteryCharging) {
  if (!batteryImage) {
    return;
  }

  int batteryY = iconFrame.origin.y - 5;
  gdraw_command_image_recolor(batteryImage, activePalette.iconFill,
                              activePalette.iconStroke);
  gdraw_command_image_draw(ctx, batteryImage,
                           GPoint(iconFrame.origin.x, batteryY));

  if (batteryCharging) {
    if (batteryChargeImage) {
      gdraw_command_image_recolor(batteryChargeImage,
                                  activePalette.iconStroke,
                                  activePalette.iconFill);
      gdraw_command_image_draw(ctx, batteryChargeImage,
                               GPoint(iconFrame.origin.x, batteryY));
    }
    return;
  }

  int width = (18 * batteryPercent + 50) / 100;
  graphics_context_set_fill_color(ctx, activePalette.iconStroke);

#ifdef PBL_COLOR
  if (batteryPercent <= 20) {
    graphics_context_set_fill_color(ctx, GColorRed);
  }
#endif

  graphics_fill_rect(ctx, GRect(iconFrame.origin.x + 3, batteryY + 8, width, 8),
                     0, GCornerNone);
}

static int single_text_height() { return fontMetrics.current.height; }
static int small_text_height() { return fontMetrics.sm.height; }
static int medium_text_height() { return fontMetrics.md.height; }
static int large_text_height() { return fontMetrics.lg.height; }

static SidebarTextMetrics fallback_current_metrics() {
  return settings.useLargeFonts ? fontMetrics.md : fontMetrics.sm;
}

static int clamp_horizontal_width(int width, SidebarRenderSpec *spec) {
  if (width < 0) {
    return 0;
  }

  return width > spec->contentWidth ? spec->contentWidth : width;
}

// inline widgets always claim their full-form width; like the vertical
// flow, over-budget configs crowd rather than compact, so the compact and
// tiny candidates only matter when a lone widget outgrows the whole bar
static int candidates_full_width(SidebarTextCandidates *candidates,
                                 GFont primaryFont,
                                 SidebarTextMetrics primaryMetrics) {
  return text_content_width(candidates->full, primaryFont, primaryMetrics) +
         SIDEBAR_TEXT_FIT_ALLOWANCE;
}

static int horizontal_content_width(SidebarContent *content,
                                    SidebarRenderSpec *spec) {
  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    return 0;
  case SIDEBAR_CONTENT_TEXT:
    return clamp_horizontal_width(
        candidates_full_width(&content->primary, currentSidebarFont,
                              fontMetrics.current),
        spec);
  case SIDEBAR_CONTENT_ICON:
    return SIDEBAR_ICON_SIZE;
  case SIDEBAR_CONTENT_ICON_VALUE:
    return clamp_horizontal_width(
        inline_icon_advance(has_icon(content)) +
            (content->isLoading
                 ? text_content_width("...", fallbackSidebarFont,
                                      fallback_current_metrics())
                 : candidates_full_width(&content->primary, currentSidebarFont,
                                         fontMetrics.current)),
        spec);
  case SIDEBAR_CONTENT_LABEL_VALUE: {
    SidebarTextCandidates joined =
        label_value_candidates(content->label, &content->primary);
    return clamp_horizontal_width(
        candidates_full_width(&joined, currentSidebarFont, fontMetrics.current),
        spec);
  }
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT: {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);
    if (content->kind == SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT) {
      // keep in sync with draw_icon_value_with_subtext: the last-resort
      // form drops the minor value
      snprintf(pairText.tiny, sizeof(pairText.tiny), "%s",
               content->primary.tiny);
    }
    int textWidth = content->isLoading
                        ? text_content_width("...", currentSidebarFont,
                                             fontMetrics.current)
                        : candidates_full_width(&pairText, currentSidebarFont,
                                                fontMetrics.current);
    return clamp_horizontal_width(
        inline_icon_advance(has_icon(content)) + textWidth, spec);
  }
  default:
    return spec->contentWidth;
  }
}

static SidebarMeasure generic_measure(SidebarContent *content,
                                      SidebarRenderSpec *spec) {
  int textHeight = single_text_height();
  // an inline icon and its text sit side by side, so the taller sets the
  // height
  int inlineHeight = (has_icon(content) && SIDEBAR_ICON_SIZE > textHeight)
                         ? SIDEBAR_ICON_SIZE
                         : textHeight;
  int height = 0;

  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    height = 0;
    break;
  case SIDEBAR_CONTENT_TEXT:
    height = spec->mode == SIDEBAR_LAYOUT_INLINE ? textHeight
                                                  : large_text_height();
    break;
  case SIDEBAR_CONTENT_ICON:
    height = SIDEBAR_ICON_SIZE;
    break;
  case SIDEBAR_CONTENT_ICON_VALUE:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = inlineHeight;
    } else {
      height = stacked_icon_advance(has_icon(content)) + textHeight;
    }
    break;
  case SIDEBAR_CONTENT_LABEL_VALUE:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = textHeight;
    } else {
      // a missing label (hidden by the "Show" setting) drops the label
      // line entirely instead of reserving an empty row
      height = textHeight;
      if (content->label != NULL && content->label[0] != '\0') {
        height += small_text_height() + SIDEBAR_BASIC_LABEL_GAP;
      }
    }
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = inlineHeight;
    } else {
      height = stacked_icon_advance(has_icon(content)) + single_text_height() +
               SIDEBAR_STACKED_VALUE_GAP + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
               SIDEBAR_STACKED_VALUE_GAP + single_text_height();
    }
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = inlineHeight;
    } else {
      height = stacked_icon_advance(has_icon(content)) + medium_text_height() +
               SIDEBAR_STACKED_VALUE_GAP + small_text_height();
    }
    break;
  default:
    height = 0;
    break;
  }

  int width = spec->mode == SIDEBAR_LAYOUT_INLINE
                  ? horizontal_content_width(content, spec)
                  : spec->contentWidth;
  return (SidebarMeasure){width, height};
}

static void draw_loading_text(GContext *ctx, int x, int visibleTop,
                              int width, GFont font,
                              SidebarTextMetrics metrics) {
  draw_literal_text_at_visible_top(ctx, "...", x, visibleTop, width, font,
                                   metrics, activePalette.text);
}

// generic_draw's layouts are kept out of line so that drawing a widget
// only puts its own layout's locals on the stack, not every layout's
// (see scratchContent below)
#define NOINLINE_DRAW __attribute__((noinline))

NOINLINE_DRAW
static void draw_icon_value(GContext *ctx, SidebarContent *content, GRect frame,
                            SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos;
  int textTop;
  GRect iconFrame;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int iconAdvance = inline_icon_advance(has_icon(content));
    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + iconAdvance;
    textWidth = inline_text_width(frame, iconAdvance);
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
  } else {
    iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                      SIDEBAR_ICON_SIZE);
    textXPos = text_x(frame, textWidth);
    textTop = frame.origin.y + stacked_icon_advance(has_icon(content));
  }

  draw_icon(ctx, content, iconFrame);

  if (content->isLoading) {
    draw_loading_text(ctx, textXPos, textTop, textWidth, fallbackSidebarFont,
                      fallback_current_metrics());
    return;
  }

  draw_text_at_visible_top_aligned(
      ctx, &content->primary, textXPos, textTop, textWidth,
      text_fit_width(spec, textWidth), currentSidebarFont, fallbackSidebarFont,
      fontMetrics.current, fallback_current_metrics(),
      spec->mode == SIDEBAR_LAYOUT_INLINE ? GTextAlignmentLeft
                                              : GTextAlignmentCenter);
}

NOINLINE_DRAW
static void draw_label_value(GContext *ctx, SidebarContent *content,
                             GRect frame, SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int labelTop = frame.origin.y;
  bool hasLabel = content->label != NULL && content->label[0] != '\0';
  int valueTop = hasLabel
                     ? labelTop + fontMetrics.sm.height + SIDEBAR_BASIC_LABEL_GAP
                     : labelTop;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    SidebarTextCandidates joined =
        label_value_candidates(content->label, &content->primary);
    int textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;

    draw_text_at_visible_top_aligned(
        ctx, &joined, frame.origin.x, textTop, frame.size.w, frame.size.w,
        currentSidebarFont, fallbackSidebarFont, fontMetrics.current,
        fallback_current_metrics(), GTextAlignmentLeft);
    return;
  }

  if (hasLabel) {
    draw_literal_text_at_visible_top(ctx, content->label, textXPos, labelTop,
                                     textWidth, smSidebarFont, fontMetrics.sm,
                                     activePalette.text);
  }
  draw_text_at_visible_top(ctx, &content->primary, textXPos, valueTop,
                           textWidth, text_fit_width(spec, textWidth),
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

static void get_icon_stack_layout(GRect frame, SidebarRenderSpec *spec,
                                  bool hasIcon, int stackHeight,
                                  GRect *iconFrame, int *textX,
                                  int *textWidth, int *textTop) {
  int renderTextWidth = render_width(spec);

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int iconAdvance = inline_icon_advance(hasIcon);
    *iconFrame = GRect(frame.origin.x,
                       frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
                       SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    *textTop = frame.origin.y + (frame.size.h - stackHeight) / 2;
    *textX = frame.origin.x + iconAdvance;
    *textWidth = inline_text_width(frame, iconAdvance);
  } else {
    *iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                       SIDEBAR_ICON_SIZE);
    *textTop = frame.origin.y + stacked_icon_advance(hasIcon);
    *textX = text_x(frame, renderTextWidth);
    *textWidth = renderTextWidth;
  }
}

NOINLINE_DRAW
static void draw_icon_value_pair_divided(GContext *ctx, SidebarContent *content,
                                         GRect frame, SidebarRenderSpec *spec) {
  GRect iconFrame;
  int textXPos;
  int textWidth;
  int textTop;
  int dividerY;
  int dividerWidth;
  int dividerX;
  int pairHeight = single_text_height() + SIDEBAR_STACKED_VALUE_GAP +
                   SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT + SIDEBAR_STACKED_VALUE_GAP +
                   single_text_height();

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);

    int iconAdvance = inline_icon_advance(has_icon(content));
    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + iconAdvance;
    textWidth = inline_text_width(frame, iconAdvance);
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;

    draw_icon(ctx, content, iconFrame);

    if (content->isLoading) {
      draw_loading_text(ctx, textXPos, textTop, textWidth, currentSidebarFont,
                        fontMetrics.current);
      return;
    }

    draw_text_at_visible_top_aligned(
        ctx, &pairText, textXPos, textTop, textWidth, textWidth,
        currentSidebarFont, fallbackSidebarFont, fontMetrics.current,
        fallback_current_metrics(), GTextAlignmentLeft);
    return;
  }

  get_icon_stack_layout(frame, spec, has_icon(content), pairHeight,
                        &iconFrame, &textXPos, &textWidth, &textTop);
  draw_icon(ctx, content, iconFrame);

  if (content->isLoading) {
    draw_loading_text(ctx, textXPos, textTop, textWidth, currentSidebarFont,
                      fontMetrics.current);
    return;
  }

  draw_text_at_visible_top(ctx, &content->primary, textXPos, textTop, textWidth,
                           text_fit_width(spec, textWidth), currentSidebarFont,
                           fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());

  dividerY = textTop + fontMetrics.current.height + SIDEBAR_STACKED_VALUE_GAP;
  dividerWidth = spec->contentWidth - SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN * 2;
  if (dividerWidth < 1) {
    dividerWidth = 1;
  }
  dividerX = frame.origin.x + (frame.size.w - spec->contentWidth) / 2 +
             SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN;

  graphics_context_set_fill_color(ctx, activePalette.text);
  graphics_fill_rect(ctx,
                     GRect(dividerX, dividerY, dividerWidth,
                           SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT),
                     0, GCornerNone);

  draw_text_at_visible_top(ctx, &content->secondary, textXPos,
                           dividerY + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
                               SIDEBAR_STACKED_VALUE_GAP,
                           textWidth, text_fit_width(spec, textWidth),
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

NOINLINE_DRAW
static void draw_icon_value_with_subtext(GContext *ctx, SidebarContent *content,
                                         GRect frame, SidebarRenderSpec *spec) {
  GRect iconFrame;
  int primaryX;
  int primaryWidth;
  int textTop;
  int stackHeight =
      medium_text_height() + SIDEBAR_STACKED_VALUE_GAP + small_text_height();

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);

    int iconAdvance = inline_icon_advance(has_icon(content));
    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    primaryX = frame.origin.x + iconAdvance;
    primaryWidth = inline_text_width(frame, iconAdvance);
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;

    // the joined pair's last resort drops the minor value ("7h", never
    // an ellipsized "0h/0...")
    snprintf(pairText.tiny, sizeof(pairText.tiny), "%s",
             content->primary.tiny);

    draw_icon(ctx, content, iconFrame);

    if (content->isLoading) {
      draw_loading_text(ctx, primaryX, textTop, primaryWidth, currentSidebarFont,
                        fontMetrics.current);
      return;
    }

    draw_text_at_visible_top_aligned(
        ctx, &pairText, primaryX, textTop, primaryWidth, primaryWidth,
        currentSidebarFont, fallbackSidebarFont, fontMetrics.current,
        fallback_current_metrics(), GTextAlignmentLeft);
    return;
  }

  get_icon_stack_layout(frame, spec, has_icon(content), stackHeight,
                        &iconFrame, &primaryX, &primaryWidth, &textTop);

  draw_icon(ctx, content, iconFrame);

  if (content->isLoading) {
    draw_loading_text(ctx, primaryX, textTop, primaryWidth, currentSidebarFont,
                      fontMetrics.current);
    return;
  }

  draw_text_at_visible_top(ctx, &content->primary, primaryX, textTop,
                           primaryWidth, text_fit_width(spec, primaryWidth),
                           mdSidebarFont, fallbackSidebarFont,
                           fontMetrics.md, fallback_current_metrics());
  draw_text_at_visible_top(
      ctx, &content->secondary, primaryX,
      textTop + fontMetrics.md.height + SIDEBAR_STACKED_VALUE_GAP, primaryWidth,
      text_fit_width(spec, primaryWidth), smSidebarFont, smSidebarFont,
      fontMetrics.sm, fontMetrics.sm);
}

static void generic_draw(GContext *ctx, SidebarContent *content, GRect frame,
                         SidebarRenderSpec *spec) {
  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    return;
  case SIDEBAR_CONTENT_TEXT: {
    int width = render_width(spec);
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      int textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
      draw_text_at_visible_top(ctx, &content->primary, text_x(frame, width),
                               textTop, width, text_fit_width(spec, width),
                               currentSidebarFont, fallbackSidebarFont,
                               fontMetrics.current, fallback_current_metrics());
      return;
    }
    draw_text_at_visible_top(ctx, &content->primary, text_x(frame, width),
                             frame.origin.y, width,
                             text_fit_width(spec, width), lgSidebarFont,
                             fallbackSidebarFont, fontMetrics.lg,
                             fallback_current_metrics());
    return;
  }
  case SIDEBAR_CONTENT_ICON:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      draw_icon(ctx, content,
                GRect(frame.origin.x,
                      frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
                      SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE));
      return;
    }
    draw_icon(ctx, content,
              GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                    SIDEBAR_ICON_SIZE));
    return;
  case SIDEBAR_CONTENT_ICON_VALUE:
    draw_icon_value(ctx, content, frame, spec);
    return;
  case SIDEBAR_CONTENT_LABEL_VALUE:
    draw_label_value(ctx, content, frame, spec);
    return;
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
    draw_icon_value_pair_divided(ctx, content, frame, spec);
    return;
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT:
    draw_icon_value_with_subtext(ctx, content, frame, spec);
    return;
  default:
    return;
  }
}

/*
 * Which of a widget's parts the user has hidden. Every widget has at most
 * one "marker" -- the part that names it: an icon, a label, or the date's
 * weekday. A few also have a "detail" that can go instead: the battery's
 * percentage, the weather's temperatures, the date's month.
 *
 * A widget never loses both parts, since that would leave it blank: if
 * both are set, the marker goes and the detail stays. The date is the
 * exception, because its calendar remains either way. The config page
 * mirrors these rules (widgetParts.ts) and only offers what applies.
 */
typedef struct {
  bool marker;
  bool detail;
} HiddenParts;

static HiddenParts hidden_parts(SidebarWidgetType type) {
  HiddenParts hidden = {
      .marker = (effectiveHideMarker >> type) & 1,
      .detail = (effectiveHideDetail >> type) & 1,
  };

  switch (type) {
  case DATE:
    break;
  case BATTERY_METER:
  case WEATHER_CURRENT:
  case WEATHER_FORECAST_TODAY:
    if (hidden.marker) {
      hidden.detail = false;
    }
    break;
  default:
    hidden.detail = false;
    break;
  }

  return hidden;
}

// strips hidden parts from a generic widget's content: a hidden marker
// drops its icon or label, and a hidden detail leaves just the icon. The
// custom widgets (battery, date) read hidden_parts() themselves.
static void hide_content_parts(SidebarWidgetType type,
                               SidebarContent *content) {
  HiddenParts hidden = hidden_parts(type);

  if (hidden.marker) {
    content->iconKind = SIDEBAR_ICON_NONE;
    content->icon = NULL;
    content->label = NULL;
  }

  if (hidden.detail &&
      (content->kind == SIDEBAR_CONTENT_ICON_VALUE ||
       content->kind == SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED)) {
    content->kind = SIDEBAR_CONTENT_ICON;
  }
}

// the content being measured or drawn. It lives here rather than on the
// stack because the text drawing underneath runs on the app's stack too:
// on flint and chalk the firmware's text rendering needs enough of it
// that a few dozen more bytes of frames above it overflowed the stack and
// froze the watch. Widgets are only ever measured and drawn one at a
// time, so a single copy is enough
static SidebarContent scratchContent;

// fetched out of line, since getContent returns through a stack temporary
// that would otherwise stay in the caller's frame for the whole draw
NOINLINE_DRAW
static void load_scratch_content(SidebarWidgetDefinition *widget) {
  scratchContent = widget->getContent();
  hide_content_parts(widget->type, &scratchContent);
}

SidebarMeasure SidebarWidgets_measure(SidebarWidgetDefinition *widget,
                                      SidebarRenderSpec *spec) {
  if (widget->measureSpecial) {
    return widget->measureSpecial(spec);
  }

  load_scratch_content(widget);
  return generic_measure(&scratchContent, spec);
}

void SidebarWidgets_draw(GContext *ctx, SidebarWidgetDefinition *widget,
                         GRect frame, SidebarRenderSpec *spec) {
  if (widget->drawSpecial) {
    widget->drawSpecial(ctx, frame, spec);
    return;
  }

  load_scratch_content(widget);
  generic_draw(ctx, &scratchContent, frame, spec);
}

static SidebarContent Empty_getContent() {
  return empty_content(SIDEBAR_CONTENT_EMPTY);
}

static void battery_text_candidates(uint8_t batteryPercent,
                                    SidebarTextCandidates *candidates) {
  if (Language_loadedId() == LANGUAGE_TR) {
    snprintf(candidates->full, sizeof(candidates->full), "%%%d",
             batteryPercent);
  } else {
    snprintf(candidates->full, sizeof(candidates->full), "%d%%",
             batteryPercent);
  }
  snprintf(candidates->compact, sizeof(candidates->compact), "%d",
           batteryPercent);
  snprintf(candidates->tiny, sizeof(candidates->tiny), "%d", batteryPercent);
}

// the battery's icon draws its charge level, so either part can stand
// alone. Charging swaps the level for a bolt, so the percentage is dropped
// while charging -- unless the icon is hidden, when it's all there is.
typedef struct {
  bool showIcon;
  bool showText;
} BatteryParts;

static BatteryParts battery_parts(bool charging) {
  HiddenParts hidden = hidden_parts(BATTERY_METER);
  bool showIcon = !hidden.marker;
  return (BatteryParts){
      .showIcon = showIcon,
      .showText = !hidden.detail && (!charging || !showIcon),
  };
}

static SidebarMeasure BatteryMeter_measure(SidebarRenderSpec *spec) {
  BatteryChargeState chargeState = battery_state_service_peek();
  BatteryParts parts = battery_parts(chargeState.is_charging);
  int iconHeight = parts.showIcon ? 14 : 0;
  int textHeight = parts.showText ? single_text_height() : 0;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int width = parts.showIcon ? SIDEBAR_ICON_SIZE : 0;

    if (parts.showText) {
      SidebarTextCandidates batteryText;
      uint8_t batteryPercent =
          (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;
      battery_text_candidates(batteryPercent, &batteryText);
      width = inline_icon_advance(parts.showIcon) +
              text_content_width(batteryText.full, currentSidebarFont,
                                 fontMetrics.current);
    }

    return (SidebarMeasure){clamp_horizontal_width(width, spec),
                            iconHeight > textHeight ? iconHeight : textHeight};
  }

  int gap = (parts.showIcon && parts.showText) ? SIDEBAR_ICON_TEXT_GAP : 0;
  return (SidebarMeasure){spec->contentWidth, iconHeight + gap + textHeight};
}

static void BatteryMeter_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec) {
  BatteryChargeState chargeState = battery_state_service_peek();
  uint8_t batteryPercent =
      (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;
  bool batteryCharging = chargeState.is_charging;
  BatteryParts parts = battery_parts(batteryCharging);
  SidebarTextCandidates batteryText;
  GRect iconFrame;
  int textWidth = render_width(spec);
  int textXPos;
  int textTop;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int iconAdvance = inline_icon_advance(parts.showIcon);
    iconFrame = GRect(frame.origin.x, frame.origin.y + (frame.size.h - 14) / 2,
                      SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + iconAdvance;
    textWidth = inline_text_width(frame, iconAdvance);
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
  } else {
    iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                      SIDEBAR_ICON_SIZE);
    textXPos = text_x(frame, textWidth);
    textTop = frame.origin.y + (parts.showIcon ? 14 + SIDEBAR_ICON_TEXT_GAP : 0);
  }

  if (parts.showIcon) {
    draw_battery_icon(ctx, iconFrame, batteryPercent, batteryCharging);
  }

  if (!parts.showText) {
    return;
  }

  battery_text_candidates(batteryPercent, &batteryText);
  draw_text_at_visible_top_aligned(
      ctx, &batteryText, textXPos, textTop, textWidth,
      text_fit_width(spec, textWidth), currentSidebarFont, fallbackSidebarFont,
      fontMetrics.current, fallback_current_metrics(),
      spec->mode == SIDEBAR_LAYOUT_INLINE ? GTextAlignmentLeft
                                              : GTextAlignmentCenter);
}

static int date_day_top_padding() {
  return settings.useLargeFonts ? DATE_CALENDAR_DAY_TOP_PADDING_LARGE
                                : DATE_CALENDAR_DAY_TOP_PADDING;
}

static int date_day_bottom_padding() {
  if (settings.useLargeFonts) {
    return DATE_CALENDAR_DAY_BOTTOM_PADDING_LARGE;
  }

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  return DATE_CALENDAR_DAY_BOTTOM_PADDING_LARGE;
#else
  return DATE_CALENDAR_DAY_BOTTOM_PADDING;
#endif
}

// the plain box is sized to the day number rather than an icon square.
// Measured against "00" so the box keeps its width as the day changes
static GSize date_plain_box_size() {
  int digitsWidth =
      text_content_width("00", currentSidebarFont, fontMetrics.current);
  return GSize(digitsWidth + DATE_PLAIN_BOX_EXTRA_WIDTH,
               fontMetrics.current.height + DATE_PLAIN_VERTICAL_PADDING * 2);
}

static int date_calendar_width() {
  if (settings.useLargeFonts) {
    return date_plain_box_size().w + DATE_STACKED_BOX_EXTRA_WIDTH;
  }

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  return 28;
#else
  return SIDEBAR_ICON_SIZE;
#endif
}

static int date_calendar_height() {
  if (settings.useLargeFonts) {
    return date_plain_box_size().h;
  }

  return date_day_top_padding() + fontMetrics.current.height +
         date_day_bottom_padding();
}

static int date_day_visible_top(int calendarHeight) {
  if (!settings.useLargeFonts) {
    return date_day_top_padding();
  }

  int textAreaHeight = calendarHeight;

  if (textAreaHeight < fontMetrics.current.height) {
    textAreaHeight = fontMetrics.current.height;
  }

  return (textAreaHeight - fontMetrics.current.height) / 2;
}

// the stacked date: weekday, calendar, month, top to bottom. The weekday
// is the date's marker and the month its detail; either or both may be
// hidden, and the calendar moves up into a hidden weekday's place
static DateLayout date_layout() {
  HiddenParts hidden = hidden_parts(DATE);
  bool showWeekday = !hidden.marker;
  bool showMonth = !hidden.detail;

  int calendarTop =
      showWeekday ? fontMetrics.current.height + SIDEBAR_ICON_TEXT_GAP : 0;
  int calendarHeight = date_calendar_height();
  int calendarBottom = calendarTop + calendarHeight;
  int monthTop = calendarBottom + SIDEBAR_ICON_TEXT_GAP;

  return (DateLayout){
      .calendarTop = calendarTop,
      .calendarHeight = calendarHeight,
      .calendarWidth = date_calendar_width(),
      .dayNumTop = calendarTop + date_day_visible_top(calendarHeight),
      .monthTop = monthTop,
      .height = showMonth ? monthTop + fontMetrics.current.height
                          : calendarBottom,
      .showWeekday = showWeekday,
      .showMonth = showMonth,
  };
}

// the width one of the inline date's words claims
static int date_inline_word_width(const char *word) {
  return text_content_width(word, currentSidebarFont, fontMetrics.current) +
         SIDEBAR_TEXT_FIT_ALLOWANCE;
}

static SidebarMeasure date_content_measure(SidebarRenderSpec *spec) {
  DateLayout metrics = date_layout();

  // inline, the stacked date is laid on its side: weekday, calendar,
  // month, left to right
  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    GSize box = date_plain_box_size();
    int height = box.h > SIDEBAR_ICON_SIZE ? box.h : SIDEBAR_ICON_SIZE;
    int width = box.w;
    if (metrics.showWeekday) {
      width += date_inline_word_width(currentDayName) + SIDEBAR_INLINE_ITEM_GAP;
    }
    if (metrics.showMonth) {
      width += SIDEBAR_INLINE_ITEM_GAP + date_inline_word_width(currentMonth);
    }
    return (SidebarMeasure){clamp_horizontal_width(width, spec), height};
  }

  return (SidebarMeasure){spec->contentWidth, metrics.height};
}

static void set_command_point(GDrawCommand *command, uint16_t pointIndex,
                              int x, int y) {
  if (command && pointIndex < gdraw_command_get_num_points(command)) {
    gdraw_command_set_point(command, pointIndex, GPoint(x, y));
  }
}

static void resize_date_calendar_image(GRect frame) {
  GDrawCommandList *commandList = gdraw_command_image_get_command_list(dateImage);
  if (!commandList || gdraw_command_list_get_num_commands(commandList) < 5) {
    return;
  }

  int left = 4;
  int top = 20;
  int right = frame.size.w * 8 - 12;
  int bottom = frame.size.h * 8 - 12;
  int bindingTop = 4;
  int bindingBottom = 52;
  int leftBindingX = (frame.size.w / 3) * 8 - 4;
  int rightBindingX = (frame.size.w * 2 / 3) * 8 + 4;
  int footHalfWidth = 16;

  GDrawCommand *body = gdraw_command_list_get_command(commandList, 0);
  set_command_point(body, 0, left, top);
  set_command_point(body, 1, right, top);
  set_command_point(body, 2, right, bottom);
  set_command_point(body, 3, left, bottom);

  GDrawCommand *leftBinding = gdraw_command_list_get_command(commandList, 1);
  set_command_point(leftBinding, 0, leftBindingX, bindingTop);
  set_command_point(leftBinding, 1, leftBindingX, bindingBottom);

  GDrawCommand *rightBinding = gdraw_command_list_get_command(commandList, 2);
  set_command_point(rightBinding, 0, rightBindingX, bindingTop);
  set_command_point(rightBinding, 1, rightBindingX, bindingBottom);

  GDrawCommand *leftFoot = gdraw_command_list_get_command(commandList, 3);
  set_command_point(leftFoot, 0, leftBindingX + footHalfWidth, bindingBottom);
  set_command_point(leftFoot, 1, leftBindingX - footHalfWidth, bindingBottom);

  GDrawCommand *rightFoot = gdraw_command_list_get_command(commandList, 4);
  set_command_point(rightFoot, 0, rightBindingX - footHalfWidth, bindingBottom);
  set_command_point(rightFoot, 1, rightBindingX + footHalfWidth, bindingBottom);

  gdraw_command_image_set_bounds_size(dateImage, GSize(frame.size.w,
                                                       frame.size.h));
}

// on dark regions icons are outlines filled with the background
// (derive_palette), so the day number inside them reads as text and takes
// the text color; inside a white-filled icon it keeps the stroke color
static GColor date_day_text_color() {
  return gcolor_equal(activePalette.iconFill, activePalette.background)
             ? activePalette.text
             : activePalette.iconStroke;
}

static void draw_date_plain_background(GContext *ctx, GRect frame) {
  graphics_context_set_fill_color(ctx, activePalette.iconStroke);
  graphics_fill_rect(ctx, frame, 2, GCornersAll);

  graphics_context_set_fill_color(ctx, activePalette.iconFill);
  graphics_fill_rect(ctx,
                     GRect(frame.origin.x + 2, frame.origin.y + 2,
                           frame.size.w - 4, frame.size.h - 4),
                     0, GCornersAll);
}

static void draw_date_calendar_background(GContext *ctx, GRect frame) {
  if (!dateImage) {
    draw_date_plain_background(ctx, frame);
    return;
  }

  resize_date_calendar_image(frame);
  gdraw_command_image_recolor(dateImage, activePalette.iconFill,
                              activePalette.iconStroke);
  gdraw_command_image_draw(ctx, dateImage, GPoint(frame.origin.x,
                                                 frame.origin.y));
}

static void draw_date_background(GContext *ctx, GRect frame) {
  if (settings.useLargeFonts) {
    draw_date_plain_background(ctx, frame);
    return;
  }

  draw_date_calendar_background(ctx, frame);
}

static void date_content_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec) {
  DateLayout metrics = date_layout();

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int x = frame.origin.x;
    int textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
    graphics_context_set_text_color(ctx, activePalette.text);

    if (metrics.showWeekday) {
      int width = date_inline_word_width(currentDayName);
      graphics_draw_text(
          ctx, currentDayName, currentSidebarFont,
          text_rect_at_visible_top(x, textTop, width, fontMetrics.current),
          GTextOverflowModeFill, GTextAlignmentLeft, NULL);
      x += width + SIDEBAR_INLINE_ITEM_GAP;
    }

    GSize box = date_plain_box_size();
    GRect calendarFrame =
        GRect(x, frame.origin.y + (frame.size.h - box.h) / 2, box.w, box.h);
    // the day number sits on the same row as the words beside it:
    // centering it within the calendar instead rounds differently from
    // centering text in the bar, leaving it a pixel high on emery
    int dayTop = textTop;

    draw_date_plain_background(ctx, calendarFrame);
    draw_literal_text_at_visible_top(ctx, currentDayNum, calendarFrame.origin.x,
                                     dayTop, calendarFrame.size.w,
                                     currentSidebarFont, fontMetrics.current,
                                     date_day_text_color());
    x += box.w + SIDEBAR_INLINE_ITEM_GAP;

    if (metrics.showMonth) {
      // draw_literal_text_at_visible_top set the day number's color
      graphics_context_set_text_color(ctx, activePalette.text);
      graphics_draw_text(
          ctx, currentMonth, currentSidebarFont,
          text_rect_at_visible_top(x, textTop, date_inline_word_width(currentMonth),
                                   fontMetrics.current),
          GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
    return;
  }

  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int calendarX =
      frame.origin.x + (frame.size.w - metrics.calendarWidth + 1) / 2;
  int calendarY = frame.origin.y + metrics.calendarTop;
  GColor dayTextColor = date_day_text_color();

  if (metrics.showWeekday) {
    draw_literal_text_at_visible_top(
        ctx, currentDayName, textXPos, frame.origin.y, textWidth,
        currentSidebarFont, fontMetrics.current, activePalette.text);
  }

  draw_date_background(ctx, GRect(calendarX, calendarY, metrics.calendarWidth,
                                  metrics.calendarHeight));

  draw_literal_text_at_visible_top(
      ctx, currentDayNum, textXPos, frame.origin.y + metrics.dayNumTop,
      textWidth, currentSidebarFont, fontMetrics.current, dayTextColor);

  if (metrics.showMonth) {
    draw_literal_text_at_visible_top(
        ctx, currentMonth, textXPos, frame.origin.y + metrics.monthTop,
        textWidth, currentSidebarFont, fontMetrics.current,
        activePalette.text);
  }
}

// rounded to the nearest degree, halves away from zero
static int celsius_to_fahrenheit(int celsius) {
  int tenths = celsius * 18 + 320;
  return (tenths + (tenths < 0 ? -5 : 5)) / 10;
}

static SidebarContent CurrentWeather_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = Weather_currentWeatherIcon;
  content.isLoading = Weather_weatherInfo.currentTemp == INT32_MIN;

  if (content.isLoading) {
    return content;
  }

  int currentTemp = Weather_weatherInfo.currentTemp;
  if (!settings.useMetric) {
    currentTemp = celsius_to_fahrenheit(currentTemp);
  }

  snprintf(content.primary.full, sizeof(content.primary.full), "%d°",
           currentTemp);
  snprintf(content.primary.compact, sizeof(content.primary.compact), "%d",
           currentTemp);
  snprintf(content.primary.tiny, sizeof(content.primary.tiny), "%d",
           currentTemp);
  return content;
}

static SidebarContent WeatherForecast_getContent() {
  SidebarContent content =
      empty_content(SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = Weather_forecastWeatherIcon;
  content.isLoading = Weather_weatherInfo.todaysHighTemp == INT32_MIN;

  if (content.isLoading) {
    return content;
  }

  int todaysHighTemp = Weather_weatherInfo.todaysHighTemp;
  int todaysLowTemp = Weather_weatherInfo.todaysLowTemp;

  if (!settings.useMetric) {
    todaysHighTemp = celsius_to_fahrenheit(todaysHighTemp);
    todaysLowTemp = celsius_to_fahrenheit(todaysLowTemp);
  }

  char high[16];
  char low[16];
  if (!settings.useLargeFonts) {
    snprintf(high, sizeof(high), "%d°", todaysHighTemp);
    snprintf(low, sizeof(low), "%d°", todaysLowTemp);
  } else {
    snprintf(high, sizeof(high), "%d", todaysHighTemp);
    snprintf(low, sizeof(low), "%d", todaysLowTemp);
  }

  content.primary = same_text_candidates(high);
  content.secondary = same_text_candidates(low);
  return content;
}

static SidebarContent WeekNumber_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = Language_strings.wordForWeek;
  content.primary = same_text_candidates(currentWeekNum);
  return content;
}

static SidebarContent Seconds_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_TEXT);
  content.primary = same_text_candidates(currentSecondsNum);
  return content;
}

static SidebarContent WorldTime1_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = settings.altclockName;
  content.primary = worldTime1Text;
  return content;
}

static SidebarContent WorldTime2_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = settings.altclock2Name;
  content.primary = worldTime2Text;
  return content;
}

static SidebarContent NextSunEvent_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;

  if (Weather_weatherInfo.sunriseMinute == INT32_MIN ||
      Weather_weatherInfo.sunsetMinute == INT32_MIN) {
    content.icon = sunriseImage;
    content.isLoading = true;
    return content;
  }

  // show whichever event comes up sooner from now
  int sunriseDelta =
      mod(Weather_weatherInfo.sunriseMinute - currentMinuteOfDay, 24 * 60);
  int sunsetDelta =
      mod(Weather_weatherInfo.sunsetMinute - currentMinuteOfDay, 24 * 60);
  bool sunriseIsNext = sunriseDelta < sunsetDelta;
  int eventMinute = sunriseIsNext ? Weather_weatherInfo.sunriseMinute
                                  : Weather_weatherInfo.sunsetMinute;

  content.icon = sunriseIsNext ? sunriseImage : sunsetImage;
  format_time_candidates(eventMinute / 60, eventMinute % 60, NULL,
                         &content.primary);
  return content;
}

static SidebarContent PrecipChance_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = precipImage;

  if (Weather_weatherInfo.precipChance == INT32_MIN) {
    content.isLoading = true;
    return content;
  }

  snprintf(content.primary.full, sizeof(content.primary.full), "%d%%",
           (int)Weather_weatherInfo.precipChance);
  snprintf(content.primary.compact, sizeof(content.primary.compact), "%d",
           (int)Weather_weatherInfo.precipChance);
  snprintf(content.primary.tiny, sizeof(content.primary.tiny), "%d",
           (int)Weather_weatherInfo.precipChance);
  return content;
}

static SidebarContent Beats_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = "@";
  content.primary = same_text_candidates(currentBeats);
  return content;
}

static SidebarContent UVIndex_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = "UV";

  if (Weather_weatherInfo.currentUVIndex != INT32_MIN) {
    char uvString[16];
    snprintf(uvString, sizeof(uvString), "%d",
             (int)Weather_weatherInfo.currentUVIndex);
    content.primary = same_text_candidates(uvString);
  } else {
    content.primary = same_text_candidates("...");
  }

  return content;
}

#ifdef PBL_HEALTH

/*
 * Today's total for a health metric, queried at most once a minute.
 * getContent runs several times per redraw (measure + draw, on every
 * region redraw), and querying daily sums in rapid succession hangs the
 * health service: first seen with sleep on basalt, then with distance on
 * chalk and basalt. The totals only change by the minute anyway.
 */
#define HEALTH_SUM_CACHE_SIZE 5

typedef struct {
  HealthMetric metric;
  int minute; // currentMinuteOfDay it was read at; -1 = empty
  int value;
} HealthSumCacheEntry;

static int cached_health_sum_today(HealthMetric metric) {
  static HealthSumCacheEntry cache[HEALTH_SUM_CACHE_SIZE] = {
      {.minute = -1}, {.minute = -1}, {.minute = -1}, {.minute = -1},
      {.minute = -1},
  };

  // slots fill in order and are never emptied, so the first slot that is
  // empty or already holds this metric is the one; there are only as many
  // summed metrics as slots, so slot 0 being reused is just a backstop
  HealthSumCacheEntry *entry = &cache[0];
  for (int i = 0; i < HEALTH_SUM_CACHE_SIZE; i++) {
    if (cache[i].minute == -1 || cache[i].metric == metric) {
      entry = &cache[i];
      break;
    }
  }

  if (entry->minute != currentMinuteOfDay || entry->metric != metric) {
    entry->metric = metric;
    entry->minute = currentMinuteOfDay;
    entry->value = is_health_metric_accessible(metric)
                       ? (int)health_service_sum_today(metric)
                       : 0;
  }

  return entry->value;
}

static bool is_widget_shown(SidebarWidgetType type) {
  for (int i = 0; i < VREGION_SLOT_CAP; i++) {
    if (settings.vRegionSlots[i] == type) {
      return true;
    }
  }
  for (int i = 0; i < HREGION_SLOT_CAP; i++) {
    if (settings.hRegionSlots[i] == type) {
      return true;
    }
  }
  return false;
}

/*
 * Reads this minute's health values for the widgets on screen, from the
 * time update rather than while drawing. Health syscalls overflow the
 * stack when made from deep in the render path, which freezes the watch
 * (distance on basalt), so the widgets' getContent should only ever hit
 * the values this caches. Each is read at most once a minute.
 */
static void refresh_health() {
  static int unitsMinute = -1;

  if (is_widget_shown(STEP_COUNTER)) {
    cached_health_sum_today(HealthMetricStepCount);
  }
  if (is_widget_shown(DISTANCE_WALKED)) {
    cached_health_sum_today(HealthMetricWalkedDistanceMeters);
    if (unitsMinute != currentMinuteOfDay) {
      unitsMinute = currentMinuteOfDay;
      distanceUnits = health_service_get_measurement_system_for_display(
          HealthMetricWalkedDistanceMeters);
    }
  }
  if (is_widget_shown(ACTIVE_CALORIES)) {
    cached_health_sum_today(HealthMetricActiveKCalories);
  }
  if (is_widget_shown(SLEEP_TIMER)) {
    cached_health_sum_today(settings.healthUseRestfulSleep
                                ? HealthMetricSleepRestfulSeconds
                                : HealthMetricSleepSeconds);
  }
}

// a count like steps or calories: in full, then as "2.7k"/"12k", then
// rounded to the nearest thousand
static void format_count_candidates(int count,
                                    SidebarTextCandidates *candidates) {
  snprintf(candidates->full, sizeof(candidates->full), "%i", count);

  if (count < 1000) {
    snprintf(candidates->compact, sizeof(candidates->compact), "%i", count);
    snprintf(candidates->tiny, sizeof(candidates->tiny), "%i", count);
    return;
  }

  int thousands = count / 1000;
  int hundreds = count / 100 % 10;

  if (count < 10000) {
    snprintf(candidates->compact, sizeof(candidates->compact), "%i%c%ik",
             thousands, settings.decimalSeparator, hundreds);
  } else {
    snprintf(candidates->compact, sizeof(candidates->compact), "%ik",
             thousands);
  }

  snprintf(candidates->tiny, sizeof(candidates->tiny), "%ik",
           (count + 500) / 1000);
}

static void format_distance_candidates(int distance,
                                       SidebarTextCandidates *candidates) {
  MeasurementSystem unitSystem = distanceUnits;

  if (unitSystem == MeasurementSystemMetric) {
    if (distance < 1000) {
      snprintf(candidates->full, sizeof(candidates->full), "%im", distance);
      snprintf(candidates->compact, sizeof(candidates->compact), "%im",
               distance);
      snprintf(candidates->tiny, sizeof(candidates->tiny), "%im", distance);
    } else {
      int kmWhole = distance / 1000;
      int kmTenths = distance / 100 % 10;
      snprintf(candidates->full, sizeof(candidates->full), "%i%c%ikm", kmWhole,
               settings.decimalSeparator, kmTenths);
      snprintf(candidates->compact, sizeof(candidates->compact), "%ikm",
               kmWhole);
      snprintf(candidates->tiny, sizeof(candidates->tiny), "%ik", kmWhole);
    }
    return;
  }

  int milesTenthsTotal = distance * 10 / 1609;
  int milesWhole = milesTenthsTotal / 10;
  int milesTenths = milesTenthsTotal % 10;

  if (milesWhole > 0) {
    snprintf(candidates->full, sizeof(candidates->full), "%i%c%imi", milesWhole,
             settings.decimalSeparator, milesTenths);
    snprintf(candidates->compact, sizeof(candidates->compact), "%imi",
             milesWhole);
    snprintf(candidates->tiny, sizeof(candidates->tiny), "%imi", milesWhole);
  } else {
    snprintf(candidates->full, sizeof(candidates->full), "%c%imi",
             settings.decimalSeparator, milesTenths);
    snprintf(candidates->compact, sizeof(candidates->compact), "%c%imi",
             settings.decimalSeparator, milesTenths);
    snprintf(candidates->tiny, sizeof(candidates->tiny), "%c%imi",
             settings.decimalSeparator, milesTenths);
  }
}

static SidebarContent StepCounter_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = stepsImage;

  format_count_candidates(cached_health_sum_today(HealthMetricStepCount),
                          &content.primary);

  return content;
}

static SidebarContent DistanceWalked_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = distanceImage;

  format_distance_candidates(
      cached_health_sum_today(HealthMetricWalkedDistanceMeters),
      &content.primary);

  return content;
}

static SidebarContent ActiveCalories_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = caloriesImage;

  format_count_candidates(cached_health_sum_today(HealthMetricActiveKCalories),
                          &content.primary);

  return content;
}

static SidebarContent SleepTimer_getContent() {
  SidebarContent content =
      empty_content(SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = sleepImage;

  HealthMetric metric = (settings.healthUseRestfulSleep)
                            ? HealthMetricSleepRestfulSeconds
                            : HealthMetricSleepSeconds;

  int sleepSeconds = cached_health_sum_today(metric);
  int sleepMinutes = sleepSeconds / 60;
  int sleepHours = sleepMinutes / 60;
  sleepMinutes %= 60;

  char hours[16];
  char minutes[16];
  snprintf(hours, sizeof(hours), "%ih", sleepHours);
  snprintf(minutes, sizeof(minutes), "%im", sleepMinutes);
  content.primary = same_text_candidates(hours);
  content.secondary = same_text_candidates(minutes);
  return content;
}

static SidebarContent HeartRate_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = heartImage;
  content.isLoading = !is_health_metric_accessible(HealthMetricHeartRateBPM);

  if (content.isLoading) {
    return content;
  }

  int heartRate = health_service_peek_current_value(HealthMetricHeartRateBPM);
  if (heartRate <= 0) {
    content.isLoading = true;
    return content;
  }

  char heartRateText[16];
  snprintf(heartRateText, sizeof(heartRateText), "%i", heartRate);
  content.primary = same_text_candidates(heartRateText);
  return content;
}

#endif

#ifdef SIDEBAR_HAS_ALARMS
static SidebarContent NextAlarm_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON_VALUE);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = alarmImage;

  // the system only tells us about the soonest enabled alarm; there's no
  // subscription, so peeking on every redraw is how we notice changes
  time_t now = time(NULL);
  time_t alarmTime;
  if (!alarm_service_peek_next(&alarmTime) || alarmTime < now) {
    content.primary = same_text_candidates("--");
    return content;
  }

  // within a day the time alone is unambiguous; further out, name the day
  struct tm alarmInfo = *localtime(&alarmTime);
  bool moreThanADayAway = alarmTime - now >= SECONDS_PER_DAY;
  format_time_candidates(
      alarmInfo.tm_hour, alarmInfo.tm_min,
      moreThanADayAway ? Language_strings.dayNames[alarmInfo.tm_wday]
                       : NULL,
      &content.primary);
  return content;
}
#endif

SidebarWidgetDefinition getSidebarWidgetByType(SidebarWidgetType type) {
  switch (type) {
  case BATTERY_METER:
    return (SidebarWidgetDefinition){BATTERY_METER, NULL, BatteryMeter_measure,
                                     BatteryMeter_draw};
  case DATE:
    return (SidebarWidgetDefinition){DATE, NULL, date_content_measure, date_content_draw};
  case WORLD_TIME_1:
    return (SidebarWidgetDefinition){WORLD_TIME_1, WorldTime1_getContent, NULL, NULL};
  case WORLD_TIME_2:
    return (SidebarWidgetDefinition){WORLD_TIME_2, WorldTime2_getContent, NULL, NULL};
  case NEXT_SUN_EVENT:
    return (SidebarWidgetDefinition){NEXT_SUN_EVENT, NextSunEvent_getContent, NULL, NULL};
  case SECONDS:
    return (SidebarWidgetDefinition){SECONDS, Seconds_getContent, NULL, NULL};
  case WEATHER_CURRENT:
    return (SidebarWidgetDefinition){WEATHER_CURRENT, CurrentWeather_getContent, NULL, NULL};
  case WEATHER_FORECAST_TODAY:
    return (SidebarWidgetDefinition){WEATHER_FORECAST_TODAY, WeatherForecast_getContent, NULL, NULL};
  case WEEK_NUMBER:
    return (SidebarWidgetDefinition){WEEK_NUMBER, WeekNumber_getContent, NULL, NULL};
  case WEATHER_UV_INDEX:
    return (SidebarWidgetDefinition){WEATHER_UV_INDEX, UVIndex_getContent, NULL, NULL};
  case WEATHER_PRECIP_CHANCE:
    return (SidebarWidgetDefinition){WEATHER_PRECIP_CHANCE, PrecipChance_getContent, NULL, NULL};
#ifdef PBL_HEALTH
  case STEP_COUNTER:
    return (SidebarWidgetDefinition){STEP_COUNTER, StepCounter_getContent, NULL, NULL};
  case DISTANCE_WALKED:
    return (SidebarWidgetDefinition){DISTANCE_WALKED, DistanceWalked_getContent, NULL, NULL};
  case ACTIVE_CALORIES:
    return (SidebarWidgetDefinition){ACTIVE_CALORIES, ActiveCalories_getContent, NULL, NULL};
  case SLEEP_TIMER:
    return (SidebarWidgetDefinition){SLEEP_TIMER, SleepTimer_getContent, NULL, NULL};
  case HEARTRATE:
    return (SidebarWidgetDefinition){HEARTRATE, HeartRate_getContent, NULL, NULL};
#endif
#ifdef SIDEBAR_HAS_ALARMS
  case NEXT_ALARM:
    return (SidebarWidgetDefinition){NEXT_ALARM, NextAlarm_getContent, NULL, NULL};
#endif
  case BEATS:
    return (SidebarWidgetDefinition){BEATS, Beats_getContent, NULL, NULL};
  default:
    return (SidebarWidgetDefinition){EMPTY, Empty_getContent, NULL, NULL};
  }
}
