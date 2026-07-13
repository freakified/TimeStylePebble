#include "sidebar_widgets.h"
#include "languages.h"
#include "settings.h"
#include "text_metrics.h"
#include "util.h"
#include "weather.h"
#include <math.h>
#include <pebble.h>
#include <string.h>

#define SIDEBAR_ICON_SIZE 25
#define SIDEBAR_ICON_TEXT_GAP 5
#define SIDEBAR_STACKED_VALUE_GAP 5
#define SIDEBAR_BASIC_LABEL_GAP 0
#define SIDEBAR_BASIC_PADDING_TOP 0
#define SIDEBAR_BASIC_PADDING_BOTTOM 0
#define SIDEBAR_TEXT_MARGIN 0
#define SIDEBAR_TEXT_OVERFLOW 5
#define SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT 1
#define SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN 2
#define SIDEBAR_HORIZONTAL_STACKED_VALUE_GAP 2
#define SIDEBAR_HORIZONTAL_STACKED_LOWER_VALUE_GAP 3
#define SIDEBAR_TEXT_FIT_ALLOWANCE 1

#define DATE_CALENDAR_DAY_TOP_PADDING 10
#define DATE_CALENDAR_DAY_BOTTOM_PADDING 4
#define DATE_CALENDAR_DAY_TOP_PADDING_LARGE 6
#define DATE_CALENDAR_DAY_BOTTOM_PADDING_LARGE 4
#define DATE_PLAIN_VERTICAL_PADDING 4

GDrawCommandImage *dateImage;
GDrawCommandImage *disconnectImage;
GDrawCommandImage *batteryImage;
GDrawCommandImage *batteryChargeImage;
GDrawCommandImage *sunriseImage;
GDrawCommandImage *sunsetImage;

#ifdef PBL_HEALTH
GDrawCommandImage *sleepImage;
GDrawCommandImage *stepsImage;
GDrawCommandImage *heartImage;
#endif

GFont smSidebarFont;
GFont mdSidebarFont;
GFont lgSidebarFont;
GFont currentSidebarFont;
GFont fallbackSidebarFont;

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
  int compactHeight;
} DateLayout;

static SidebarFontMetrics fontMetrics;

char currentDayName[8];
char currentDayNum[5];
char currentMonth[8];
char currentWeekNum[5];
char currentSecondsNum[5];
char currentBeats[5];

// current world times, formatted for display
static SidebarTextCandidates worldTime1Text;
static SidebarTextCandidates worldTime2Text;

// minutes since local midnight, for the sunrise/sunset widget
static int currentMinuteOfDay;

static SidebarContent Empty_getContent();
static SidebarMeasure BatteryMeter_measure(SidebarRenderSpec *spec);
static void BatteryMeter_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec);
static SidebarContent Date_getContent();
static SidebarMeasure date_content_measure(SidebarRenderSpec *spec);
static void date_content_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec);
static SidebarContent CurrentWeather_getContent();
static SidebarContent WeatherForecast_getContent();
static SidebarContent BTDisconnect_getContent();
static SidebarContent WeekNumber_getContent();
static SidebarContent Seconds_getContent();
static SidebarContent WorldTime1_getContent();
static SidebarContent WorldTime2_getContent();
static SidebarContent NextSunEvent_getContent();
static SidebarContent Beats_getContent();
static SidebarContent UVIndex_getContent();
static SidebarContent Wind_getContent();
static SidebarContent PrecipChance_getContent();

#ifdef PBL_HEALTH
static SidebarContent StepCounter_getContent();
static SidebarContent SleepTimer_getContent();
static SidebarContent HeartRate_getContent();
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
  disconnectImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
  batteryImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
  batteryChargeImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  sunriseImage = gdraw_command_image_create_with_resource(RESOURCE_ID_SUNRISE);
  sunsetImage = gdraw_command_image_create_with_resource(RESOURCE_ID_SUNSET);

#ifdef PBL_HEALTH
  sleepImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_SLEEP);
  stepsImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_STEPS);
  heartImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_HEART);
#endif
}

void SidebarWidgets_deinit() {
  gdraw_command_image_destroy(dateImage);
  gdraw_command_image_destroy(disconnectImage);
  gdraw_command_image_destroy(batteryImage);
  gdraw_command_image_destroy(batteryChargeImage);
  gdraw_command_image_destroy(sunriseImage);
  gdraw_command_image_destroy(sunsetImage);

#ifdef PBL_HEALTH
  gdraw_command_image_destroy(stepsImage);
  gdraw_command_image_destroy(sleepImage);
  gdraw_command_image_destroy(heartImage);
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

int mod(int a, int b) {
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
      differentDate ? dayNames[settings.languageId][altTime.tm_wday] : NULL,
      out);
}

void SidebarWidgets_updateTime(struct tm *timeInfo) {
  strftime(currentDayNum, 3, "%e", timeInfo);
  if (currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }

  strftime(currentWeekNum, 3, "%V", timeInfo);

  strncpy(currentDayName, dayNames[settings.languageId][timeInfo->tm_wday],
          sizeof(currentDayName));
  currentDayName[sizeof(currentDayName) - 1] = '\0';
  strncpy(currentMonth, monthNames[settings.languageId][timeInfo->tm_mon],
          sizeof(currentMonth));
  currentMonth[sizeof(currentMonth) - 1] = '\0';

  strftime(currentSecondsNum, 4, ":%S", timeInfo);

  currentMinuteOfDay = timeInfo->tm_hour * 60 + timeInfo->tm_min;

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
                                                int contentWidth,
                                                bool compact) {
  SidebarWidgets_updateFonts();
  return (SidebarRenderSpec){
      .mode = mode,
      .contentWidth = contentWidth,
      .compact = compact,
  };
}

// the palette widgets draw with; the region renderer sets this before
// drawing, the same way it sets colors on the GContext
static SidebarPalette activePalette;

void SidebarWidgets_setActivePalette(const SidebarPalette *palette) {
  activePalette = *palette;
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

static int icon_text_gap(SidebarRenderSpec *spec) {
  return spec->compact ? 5 : SIDEBAR_ICON_TEXT_GAP;
}

static int horizontal_item_gap() { return 3; }

static int stacked_value_gap(SidebarRenderSpec *spec) {
  return spec->compact ? 3 : SIDEBAR_STACKED_VALUE_GAP;
}

static int basic_label_gap(SidebarRenderSpec *spec) {
  return spec->compact ? 3 : SIDEBAR_BASIC_LABEL_GAP;
}

static int basic_padding_top(SidebarRenderSpec *spec) {
  return spec->compact ? 0 : SIDEBAR_BASIC_PADDING_TOP;
}

static int basic_padding_bottom(SidebarRenderSpec *spec) {
  return spec->compact ? 2 : SIDEBAR_BASIC_PADDING_BOTTOM;
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

static GRect fit_rect(GRect drawRect) {
  int margin =
      drawRect.size.w > SIDEBAR_TEXT_MARGIN * 2 ? SIDEBAR_TEXT_MARGIN : 0;
  return GRect(drawRect.origin.x + margin, drawRect.origin.y,
               drawRect.size.w - margin * 2, drawRect.size.h);
}

static GRect text_rect_at_visible_top(int x, int visibleTop, int width,
                                      SidebarTextMetrics metrics) {
  return GRect(x, text_draw_y(metrics, visibleTop), width,
               text_rect_height(metrics));
}

static void draw_text_at_visible_top(GContext *ctx,
                                     SidebarTextCandidates *candidates, int x,
                                     int visibleTop, int width, int fitWidth,
                                     GFont primaryFont, GFont fallbackFont,
                                     SidebarTextMetrics primaryMetrics,
                                     SidebarTextMetrics fallbackMetrics) {
  GFont selectedFont = primaryFont;
  const char *text = pick_text(candidates, primaryFont, fallbackFont,
                               fit_rect(GRect(x, visibleTop, fitWidth,
                                              text_rect_height(primaryMetrics))),
                               &selectedFont);
  SidebarTextMetrics selectedMetrics =
      (selectedFont == primaryFont) ? primaryMetrics : fallbackMetrics;
  GRect drawRect =
      text_rect_at_visible_top(x, visibleTop, width, selectedMetrics);

  graphics_context_set_text_color(ctx, activePalette.text);
  graphics_draw_text(ctx, text, selectedFont, drawRect, GTextOverflowModeFill,
                     GTextAlignmentCenter, NULL);
}

static void draw_text_at_visible_top_aligned(
    GContext *ctx, SidebarTextCandidates *candidates, int x, int visibleTop,
    int width, int fitWidth, GFont primaryFont, GFont fallbackFont,
    SidebarTextMetrics primaryMetrics, SidebarTextMetrics fallbackMetrics,
    GTextAlignment alignment) {
  GFont selectedFont = primaryFont;
  const char *text = pick_text(candidates, primaryFont, fallbackFont,
                               fit_rect(GRect(x, visibleTop, fitWidth,
                                              text_rect_height(primaryMetrics))),
                               &selectedFont);
  SidebarTextMetrics selectedMetrics =
      (selectedFont == primaryFont) ? primaryMetrics : fallbackMetrics;
  GRect drawRect =
      text_rect_at_visible_top(x, visibleTop, width, selectedMetrics);

  graphics_context_set_text_color(ctx, activePalette.text);
  graphics_draw_text(ctx, text, selectedFont, drawRect, GTextOverflowModeFill,
                     alignment, NULL);
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

static void draw_icon(GContext *ctx, SidebarContent *content, GRect iconFrame) {
  if (content->iconKind != SIDEBAR_ICON_IMAGE || !content->icon) {
    return;
  }

  gdraw_command_image_recolor(content->icon, activePalette.iconFill,
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

  int width = roundf(18 * batteryPercent / 100.0f);
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

static int candidates_content_width(SidebarTextCandidates *candidates,
                                    GFont font,
                                    SidebarTextMetrics metrics) {
  return text_content_width(candidates->full, font, metrics);
}

static int candidates_fit_width(SidebarTextCandidates *candidates,
                                GFont primaryFont,
                                SidebarTextMetrics primaryMetrics,
                                int maxWidth) {
  int width = text_content_width(candidates->full, primaryFont,
                                 primaryMetrics);
  int fitWidth = width + SIDEBAR_TEXT_FIT_ALLOWANCE;
  if (fitWidth <= maxWidth) {
    return fitWidth;
  }

  width = text_content_width(candidates->compact, primaryFont,
                             primaryMetrics);
  fitWidth = width + SIDEBAR_TEXT_FIT_ALLOWANCE;
  if (fitWidth <= maxWidth) {
    return fitWidth;
  }

  return text_content_width(candidates->tiny, primaryFont, primaryMetrics);
}

static int horizontal_pair_stack_height() {
  return fontMetrics.sm.height + SIDEBAR_HORIZONTAL_STACKED_VALUE_GAP +
         SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
         SIDEBAR_HORIZONTAL_STACKED_LOWER_VALUE_GAP + fontMetrics.sm.height;
}

static int horizontal_pair_stack_width(SidebarContent *content) {
  int primaryWidth =
      text_content_width(content->primary.tiny, smSidebarFont, fontMetrics.sm);
  int secondaryWidth =
      text_content_width(content->secondary.tiny, smSidebarFont, fontMetrics.sm);
  int textWidth = primaryWidth > secondaryWidth ? primaryWidth : secondaryWidth;
  int dividerWidth = textWidth > SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN * 2
                         ? textWidth
                         : SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN * 2;

  return SIDEBAR_ICON_SIZE + horizontal_item_gap() +
         (textWidth > dividerWidth ? textWidth : dividerWidth);
}

static bool should_stack_horizontal_pair(SidebarContent *content, GRect frame,
                                         SidebarRenderSpec *spec,
                                         int textWidth) {
  if (spec->mode != SIDEBAR_LAYOUT_INLINE || content->isLoading ||
      textWidth < 1 || horizontal_pair_stack_height() > frame.size.h) {
    return false;
  }

  SidebarTextCandidates pairText =
      pair_text_candidates(&content->primary, &content->secondary);
  GRect fitRect = fit_rect(GRect(0, 0, textWidth, fontMetrics.current.height));

  return !text_fits(pairText.tiny, fallbackSidebarFont, fitRect);
}

static int horizontal_content_width(SidebarContent *content,
                                    SidebarRenderSpec *spec) {
  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    return 0;
  case SIDEBAR_CONTENT_TEXT:
    return clamp_horizontal_width(
        candidates_fit_width(&content->primary, currentSidebarFont,
                             fontMetrics.current, spec->contentWidth),
        spec);
  case SIDEBAR_CONTENT_ICON:
    return SIDEBAR_ICON_SIZE;
  case SIDEBAR_CONTENT_ICON_VALUE:
    return clamp_horizontal_width(
        SIDEBAR_ICON_SIZE + horizontal_item_gap() +
            (content->isLoading
                 ? text_content_width("...", fallbackSidebarFont,
                                      fallback_current_metrics())
                 : candidates_fit_width(
                       &content->primary, currentSidebarFont,
                       fontMetrics.current, spec->contentWidth -
                                                SIDEBAR_ICON_SIZE -
                                                horizontal_item_gap())),
        spec);
  case SIDEBAR_CONTENT_LABEL_VALUE:
    return clamp_horizontal_width(
        text_content_width(content->label, currentSidebarFont,
                           fontMetrics.current) +
            horizontal_item_gap() +
            candidates_fit_width(
                &content->primary, currentSidebarFont, fontMetrics.current,
                spec->contentWidth -
                    text_content_width(content->label, currentSidebarFont,
                                       fontMetrics.current) -
                    horizontal_item_gap()),
        spec);
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT: {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);
    int textWidth = content->isLoading
                        ? text_content_width("...", currentSidebarFont,
                                             fontMetrics.current)
                        : candidates_fit_width(
                              &pairText, currentSidebarFont,
                              fontMetrics.current,
                              spec->contentWidth - SIDEBAR_ICON_SIZE -
                                  horizontal_item_gap());
    int pairTextWidth = spec->contentWidth - SIDEBAR_ICON_SIZE -
                        horizontal_item_gap();
    if (spec->compact) {
      pairTextWidth = spec->contentWidth / 2 - SIDEBAR_ICON_SIZE -
                      horizontal_item_gap();
    }

    if (content->kind == SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED &&
        !content->isLoading &&
        textWidth > pairTextWidth) {
      return clamp_horizontal_width(horizontal_pair_stack_width(content), spec);
    }

    return clamp_horizontal_width(
        SIDEBAR_ICON_SIZE + horizontal_item_gap() + textWidth, spec);
  }
  default:
    return spec->contentWidth;
  }
}

static SidebarMeasure generic_measure(SidebarContent *content,
                                      SidebarRenderSpec *spec) {
  int textHeight = single_text_height();
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
      height = SIDEBAR_ICON_SIZE > textHeight ? SIDEBAR_ICON_SIZE : textHeight;
    } else {
      height = SIDEBAR_ICON_SIZE + icon_text_gap(spec) + textHeight;
    }
    break;
  case SIDEBAR_CONTENT_LABEL_VALUE:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = textHeight;
    } else {
      height = basic_padding_top(spec) + small_text_height() +
               basic_label_gap(spec) + textHeight + basic_padding_bottom(spec);
    }
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = SIDEBAR_ICON_SIZE > textHeight ? SIDEBAR_ICON_SIZE : textHeight;
    } else {
      height = SIDEBAR_ICON_SIZE + icon_text_gap(spec) + single_text_height() +
               stacked_value_gap(spec) + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
               stacked_value_gap(spec) + single_text_height();
    }
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT:
    if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
      height = SIDEBAR_ICON_SIZE > textHeight ? SIDEBAR_ICON_SIZE : textHeight;
    } else {
      height = SIDEBAR_ICON_SIZE + icon_text_gap(spec) + medium_text_height() +
               stacked_value_gap(spec) + small_text_height();
    }
    break;
  case SIDEBAR_CONTENT_DATE:
    return date_content_measure(spec);
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

static void draw_icon_value(GContext *ctx, SidebarContent *content, GRect frame,
                            SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos;
  int textTop;
  GRect iconFrame;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    textWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
  } else {
    iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                      SIDEBAR_ICON_SIZE);
    textXPos = text_x(frame, textWidth);
    textTop = frame.origin.y + SIDEBAR_ICON_SIZE + icon_text_gap(spec);
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

static void draw_label_value(GContext *ctx, SidebarContent *content,
                             GRect frame, SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int labelTop = frame.origin.y + basic_padding_top(spec);
  int valueTop = labelTop + fontMetrics.sm.height + basic_label_gap(spec);

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
    int labelWidth =
        text_content_width(content->label, currentSidebarFont,
                           fontMetrics.current);
    int valueX = frame.origin.x + labelWidth + horizontal_item_gap();
    int valueWidth = frame.size.w - labelWidth - horizontal_item_gap();

    if (valueWidth < 1) {
      valueWidth = 1;
    }

    draw_literal_text_at_visible_top(ctx, content->label, frame.origin.x,
                                     textTop, labelWidth, currentSidebarFont,
                                     fontMetrics.current,
                                     activePalette.text);
    draw_text_at_visible_top_aligned(
        ctx, &content->primary, valueX, textTop, valueWidth, valueWidth,
        currentSidebarFont, fallbackSidebarFont, fontMetrics.current,
        fallback_current_metrics(), GTextAlignmentLeft);
    return;
  }

  draw_literal_text_at_visible_top(ctx, content->label, textXPos, labelTop,
                                   textWidth, smSidebarFont, fontMetrics.sm,
                                   activePalette.text);
  draw_text_at_visible_top(ctx, &content->primary, textXPos, valueTop,
                           textWidth, text_fit_width(spec, textWidth),
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

static void get_icon_stack_layout(GRect frame, SidebarRenderSpec *spec,
                                  int stackHeight, GRect *iconFrame, int *textX,
                                  int *textWidth, int *textTop) {
  int renderTextWidth = render_width(spec);

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    *iconFrame = GRect(frame.origin.x,
                       frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
                       SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    *textTop = frame.origin.y + (frame.size.h - stackHeight) / 2;
    *textX = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    *textWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
  } else {
    *iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                       SIDEBAR_ICON_SIZE);
    *textTop = frame.origin.y + SIDEBAR_ICON_SIZE + icon_text_gap(spec);
    *textX = text_x(frame, renderTextWidth);
    *textWidth = renderTextWidth;
  }
}

static void draw_icon_value_pair_divided(GContext *ctx, SidebarContent *content,
                                         GRect frame, SidebarRenderSpec *spec) {
  GRect iconFrame;
  int textXPos;
  int textWidth;
  int textTop;
  int dividerY;
  int dividerWidth;
  int dividerX;
  int pairHeight = single_text_height() + stacked_value_gap(spec) +
                   SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT + stacked_value_gap(spec) +
                   single_text_height();

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);
    int stackHeight = horizontal_pair_stack_height();

    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    textWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;

    if (textWidth < 1) {
      textWidth = 1;
    }

    draw_icon(ctx, content, iconFrame);

    if (content->isLoading) {
      draw_loading_text(ctx, textXPos, textTop, textWidth, currentSidebarFont,
                        fontMetrics.current);
      return;
    }

    if (should_stack_horizontal_pair(content, frame, spec, textWidth)) {
      int stackTop = frame.origin.y + (frame.size.h - stackHeight) / 2;
      int dividerY = stackTop + fontMetrics.sm.height +
                     SIDEBAR_HORIZONTAL_STACKED_VALUE_GAP;
      int dividerWidth = textWidth - SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN * 2;
      int dividerX = textXPos + SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN;

      if (dividerWidth < 1) {
        dividerWidth = 1;
      }

      draw_text_at_visible_top_aligned(
          ctx, &content->primary, textXPos, stackTop, textWidth, textWidth,
          smSidebarFont, smSidebarFont, fontMetrics.sm, fontMetrics.sm,
          GTextAlignmentCenter);

      graphics_context_set_fill_color(ctx, activePalette.text);
      graphics_fill_rect(ctx,
                         GRect(dividerX, dividerY, dividerWidth,
                               SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT),
                         0, GCornerNone);

      draw_text_at_visible_top_aligned(
          ctx, &content->secondary, textXPos,
          dividerY + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
              SIDEBAR_HORIZONTAL_STACKED_LOWER_VALUE_GAP,
          textWidth, textWidth, smSidebarFont, smSidebarFont, fontMetrics.sm,
          fontMetrics.sm, GTextAlignmentCenter);
      return;
    }

    draw_text_at_visible_top_aligned(
        ctx, &pairText, textXPos, textTop, textWidth, textWidth,
        currentSidebarFont, fallbackSidebarFont, fontMetrics.current,
        fallback_current_metrics(), GTextAlignmentLeft);
    return;
  }

  get_icon_stack_layout(frame, spec, pairHeight, &iconFrame, &textXPos,
                        &textWidth, &textTop);
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

  dividerY = textTop + fontMetrics.current.height + stacked_value_gap(spec);
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
                               stacked_value_gap(spec),
                           textWidth, text_fit_width(spec, textWidth),
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

static void draw_icon_value_with_subtext(GContext *ctx, SidebarContent *content,
                                         GRect frame, SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  GRect iconFrame;
  int primaryX;
  int primaryWidth;
  int textTop;
  int stackHeight =
      medium_text_height() + stacked_value_gap(spec) + small_text_height();

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    SidebarTextCandidates pairText =
        pair_text_candidates(&content->primary, &content->secondary);

    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    primaryX = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    primaryWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;

    if (primaryWidth < 1) {
      primaryWidth = 1;
    }

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

  get_icon_stack_layout(frame, spec, stackHeight, &iconFrame, &primaryX,
                        &primaryWidth, &textTop);
  (void)textWidth;

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
      textTop + fontMetrics.md.height + stacked_value_gap(spec), primaryWidth,
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
  case SIDEBAR_CONTENT_DATE:
    date_content_draw(ctx, frame, spec);
    return;
  default:
    return;
  }
}

SidebarMeasure SidebarWidgets_measure(SidebarWidgetDefinition *widget,
                                      SidebarRenderSpec *spec) {
  if (widget->measureSpecial) {
    return widget->measureSpecial(spec);
  }

  SidebarContent content = widget->getContent();
  return generic_measure(&content, spec);
}

void SidebarWidgets_draw(GContext *ctx, SidebarWidgetDefinition *widget,
                         GRect frame, SidebarRenderSpec *spec) {
  if (widget->drawSpecial) {
    widget->drawSpecial(ctx, frame, spec);
    return;
  }

  SidebarContent content = widget->getContent();
  generic_draw(ctx, &content, frame, spec);
}

static SidebarContent Empty_getContent() {
  return empty_content(SIDEBAR_CONTENT_EMPTY);
}

static void battery_text_candidates(uint8_t batteryPercent,
                                    SidebarTextCandidates *candidates) {
  if (settings.languageId == LANGUAGE_TR) {
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

static SidebarMeasure BatteryMeter_measure(SidebarRenderSpec *spec) {
  BatteryChargeState chargeState = battery_state_service_peek();

  if (chargeState.is_charging || !settings.showBatteryPct) {
    return (SidebarMeasure){spec->mode == SIDEBAR_LAYOUT_INLINE
                                ? SIDEBAR_ICON_SIZE
                                : spec->contentWidth,
                            14};
  }

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int textHeight = single_text_height();
    SidebarTextCandidates batteryText;
    uint8_t batteryPercent =
        (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;
    battery_text_candidates(batteryPercent, &batteryText);
    int width = SIDEBAR_ICON_SIZE + horizontal_item_gap() +
                candidates_content_width(&batteryText, currentSidebarFont,
                                         fontMetrics.current);
    return (SidebarMeasure){clamp_horizontal_width(width, spec),
                            14 > textHeight ? 14 : textHeight};
  }

  return (SidebarMeasure){spec->contentWidth,
                          14 + icon_text_gap(spec) + single_text_height()};
}

static void BatteryMeter_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec) {
  BatteryChargeState chargeState = battery_state_service_peek();
  uint8_t batteryPercent =
      (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;
  bool batteryCharging = chargeState.is_charging;
  SidebarTextCandidates batteryText;
  GRect iconFrame;
  int textWidth = render_width(spec);
  int textXPos;
  int textTop;

  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    iconFrame = GRect(frame.origin.x, frame.origin.y + (frame.size.h - 14) / 2,
                      SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    textWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
    textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
  } else {
    iconFrame = GRect(icon_x(frame), frame.origin.y, SIDEBAR_ICON_SIZE,
                      SIDEBAR_ICON_SIZE);
    textXPos = text_x(frame, textWidth);
    textTop = frame.origin.y + 14 + icon_text_gap(spec);
  }

  draw_battery_icon(ctx, iconFrame, batteryPercent, batteryCharging);

  if (batteryCharging || !settings.showBatteryPct) {
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

static SidebarContent Date_getContent() {
  return empty_content(SIDEBAR_CONTENT_DATE);
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

static int date_calendar_width() {
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  return 28;
#else
  return settings.useLargeFonts ? 26 : SIDEBAR_ICON_SIZE;
#endif
}

static int date_calendar_height() {
  if (settings.useLargeFonts) {
    return fontMetrics.current.height + DATE_PLAIN_VERTICAL_PADDING * 2;
  }

  return date_day_top_padding() + fontMetrics.current.height +
         date_day_bottom_padding();
}

static int date_day_visible_top(int calendarHeight) {
  if (!settings.useLargeFonts) {
    return date_day_top_padding();
  }

  int textAreaTop = 0;
  int textAreaHeight = calendarHeight;

  if (textAreaHeight < fontMetrics.current.height) {
    textAreaHeight = fontMetrics.current.height;
  }

  return textAreaTop + (textAreaHeight - fontMetrics.current.height) / 2;
}

static DateLayout date_layout(SidebarRenderSpec *spec) {
  int weekdayToCalendarGap = icon_text_gap(spec);
  int calendarToMonthGap = icon_text_gap(spec);
  int calendarTop = fontMetrics.current.height + weekdayToCalendarGap;
  int calendarHeight = date_calendar_height();
  int calendarWidth = date_calendar_width();
  int monthTop = calendarTop + calendarHeight + calendarToMonthGap;

  return (DateLayout){
      .calendarTop = calendarTop,
      .calendarHeight = calendarHeight,
      .calendarWidth = calendarWidth,
      .dayNumTop = calendarTop + date_day_visible_top(calendarHeight),
      .monthTop = monthTop,
      .height = monthTop + fontMetrics.current.height,
      .compactHeight = calendarTop + calendarHeight,
  };
}

static SidebarMeasure date_content_measure(SidebarRenderSpec *spec) {
  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    int textHeight = single_text_height();
    int width = SIDEBAR_ICON_SIZE + horizontal_item_gap() +
                text_content_width(currentDayName, currentSidebarFont,
                                   fontMetrics.current);
    return (SidebarMeasure){clamp_horizontal_width(width, spec),
                            SIDEBAR_ICON_SIZE > textHeight ? SIDEBAR_ICON_SIZE
                                                            : textHeight};
  }

  DateLayout metrics = date_layout(spec);
  return (SidebarMeasure){spec->contentWidth, spec->compact
                                                  ? metrics.compactHeight
                                                  : metrics.height};
}

static GPoint precise_point(int x, int y) { return GPoint(x, y); }

static void set_command_point(GDrawCommand *command, uint16_t pointIndex,
                              int x, int y) {
  if (command && pointIndex < gdraw_command_get_num_points(command)) {
    gdraw_command_set_point(command, pointIndex, precise_point(x, y));
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

static void draw_date_calendar_background_fallback(GContext *ctx, GRect frame) {
  int bodyTop = frame.origin.y + 2;
  int bodyHeight = frame.size.h - 2;
  int bindingY = frame.origin.y;
  int bindingBottom = frame.origin.y + 6;
  int leftBindingX = frame.origin.x + frame.size.w / 3;
  int rightBindingX = frame.origin.x + frame.size.w * 2 / 3;

  graphics_context_set_fill_color(ctx, activePalette.iconFill);
  graphics_fill_rect(ctx, GRect(frame.origin.x, bodyTop, frame.size.w,
                                bodyHeight),
                     0, GCornerNone);

  graphics_context_set_stroke_color(ctx, activePalette.iconStroke);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_rect(ctx, GRect(frame.origin.x, bodyTop, frame.size.w,
                                bodyHeight));

  graphics_draw_line(ctx, GPoint(leftBindingX, bindingY),
                     GPoint(leftBindingX, bindingBottom));
  graphics_draw_line(ctx, GPoint(rightBindingX, bindingY),
                     GPoint(rightBindingX, bindingBottom));
  graphics_draw_line(ctx, GPoint(leftBindingX - 2, bindingBottom),
                     GPoint(leftBindingX + 2, bindingBottom));
  graphics_draw_line(ctx, GPoint(rightBindingX - 2, bindingBottom),
                     GPoint(rightBindingX + 2, bindingBottom));
  graphics_context_set_stroke_width(ctx, 1);
}

static void draw_date_calendar_background(GContext *ctx, GRect frame) {
  if (!dateImage) {
    draw_date_calendar_background_fallback(ctx, frame);
    return;
  }

  resize_date_calendar_image(frame);
  gdraw_command_image_recolor(dateImage, activePalette.iconFill,
                              activePalette.iconStroke);
  gdraw_command_image_draw(ctx, dateImage, GPoint(frame.origin.x,
                                                 frame.origin.y));
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

static void draw_date_background(GContext *ctx, GRect frame) {
  if (settings.useLargeFonts) {
    draw_date_plain_background(ctx, frame);
    return;
  }

  draw_date_calendar_background(ctx, frame);
}

static void date_content_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec) {
  if (spec->mode == SIDEBAR_LAYOUT_INLINE) {
    GRect calendarFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    int textX = frame.origin.x + SIDEBAR_ICON_SIZE + horizontal_item_gap();
    int textWidth = frame.size.w - SIDEBAR_ICON_SIZE - horizontal_item_gap();
    int textTop = frame.origin.y + (frame.size.h - single_text_height()) / 2;
    int dayTop = calendarFrame.origin.y +
                 (calendarFrame.size.h - fontMetrics.current.height) / 2;

    draw_date_background(ctx, calendarFrame);
    draw_literal_text_at_visible_top(ctx, currentDayNum, calendarFrame.origin.x,
                                     dayTop, calendarFrame.size.w,
                                     currentSidebarFont, fontMetrics.current,
                                     activePalette.iconStroke);
    graphics_context_set_text_color(ctx, activePalette.text);
    graphics_draw_text(
        ctx, currentDayName, currentSidebarFont,
        text_rect_at_visible_top(textX, textTop, textWidth, fontMetrics.current),
        GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    return;
  }

  DateLayout metrics = date_layout(spec);
  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int calendarX =
      frame.origin.x + (frame.size.w - metrics.calendarWidth + 1) / 2;
  int calendarY = frame.origin.y + metrics.calendarTop;
  GColor dayTextColor = activePalette.iconStroke;

  draw_literal_text_at_visible_top(
      ctx, currentDayName, textXPos, frame.origin.y, textWidth,
      currentSidebarFont, fontMetrics.current, activePalette.text);

  draw_date_background(ctx, GRect(calendarX, calendarY, metrics.calendarWidth,
                                  metrics.calendarHeight));

  draw_literal_text_at_visible_top(
      ctx, currentDayNum, textXPos, frame.origin.y + metrics.dayNumTop,
      textWidth, currentSidebarFont, fontMetrics.current, dayTextColor);

  if (!spec->compact) {
    draw_literal_text_at_visible_top(
        ctx, currentMonth, textXPos, frame.origin.y + metrics.monthTop,
        textWidth, currentSidebarFont, fontMetrics.current,
        activePalette.text);
  }
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
    currentTemp = roundf(currentTemp * 1.8f + 32);
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
    todaysHighTemp = roundf(todaysHighTemp * 1.8f + 32);
    todaysLowTemp = roundf(todaysLowTemp * 1.8f + 32);
  }

  char high[8];
  char low[8];
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

static SidebarContent BTDisconnect_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_ICON);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = disconnectImage;
  return content;
}

static SidebarContent WeekNumber_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = wordForWeek[settings.languageId];
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

static SidebarContent Wind_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = "WIND";

  if (Weather_weatherInfo.windSpeed == INT32_MIN) {
    content.primary = same_text_candidates("...");
    return content;
  }

  int windSpeed = Weather_weatherInfo.windSpeed;
  const char *unit = "kph";
  if (!settings.useMetric) {
    windSpeed = roundf(windSpeed / 1.609344f);
    unit = "mph";
  }

  snprintf(content.primary.full, sizeof(content.primary.full), "%d%s",
           windSpeed, unit);
  snprintf(content.primary.compact, sizeof(content.primary.compact), "%d",
           windSpeed);
  snprintf(content.primary.tiny, sizeof(content.primary.tiny), "%d",
           windSpeed);
  return content;
}

static SidebarContent PrecipChance_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = "RAIN";

  if (Weather_weatherInfo.precipChance == INT32_MIN) {
    content.primary = same_text_candidates("...");
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

static void format_step_candidates(int steps,
                                   SidebarTextCandidates *candidates) {
  snprintf(candidates->full, sizeof(candidates->full), "%i", steps);

  if (steps < 1000) {
    snprintf(candidates->compact, sizeof(candidates->compact), "%i", steps);
    snprintf(candidates->tiny, sizeof(candidates->tiny), "%i", steps);
    return;
  }

  int stepsThousands = steps / 1000;
  int stepsHundreds = steps / 100 % 10;

  if (steps < 10000) {
    snprintf(candidates->compact, sizeof(candidates->compact), "%i%c%ik",
             stepsThousands, settings.decimalSeparator, stepsHundreds);
  } else {
    snprintf(candidates->compact, sizeof(candidates->compact), "%ik",
             stepsThousands);
  }

  snprintf(candidates->tiny, sizeof(candidates->tiny), "%ik",
           (steps + 500) / 1000);
}

static void format_distance_candidates(int distance,
                                       SidebarTextCandidates *candidates) {
  MeasurementSystem unitSystem =
      health_service_get_measurement_system_for_display(
          HealthMetricWalkedDistanceMeters);

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

  if (settings.healthUseDistance) {
    int distance = 0;
    if (is_health_metric_accessible(HealthMetricWalkedDistanceMeters)) {
      distance =
          (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
    }
    format_distance_candidates(distance, &content.primary);
  } else {
    int steps = 0;
    if (is_health_metric_accessible(HealthMetricStepCount)) {
      steps = (int)health_service_sum_today(HealthMetricStepCount);
    }
    format_step_candidates(steps, &content.primary);
  }

  return content;
}

static SidebarContent SleepTimer_getContent() {
  SidebarContent content =
      empty_content(SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT);
  content.iconKind = SIDEBAR_ICON_IMAGE;
  content.icon = sleepImage;

  HealthActivityMask metric = (settings.healthUseRestfulSleep)
                                  ? HealthMetricSleepRestfulSeconds
                                  : HealthMetricSleepSeconds;

  // getContent runs several times per redraw (measure + draw), and querying
  // the sleep metric in rapid succession hangs basalt's health service, so
  // only ask once per minute
  static int sleepCacheMinute = -1;
  static HealthMetric sleepCacheMetric;
  static int sleepCacheSeconds;

  if (sleepCacheMinute != currentMinuteOfDay || sleepCacheMetric != metric) {
    sleepCacheSeconds = is_health_metric_accessible(metric)
                            ? (int)health_service_sum_today(metric)
                            : 0;
    sleepCacheMinute = currentMinuteOfDay;
    sleepCacheMetric = metric;
  }

  int sleepSeconds = sleepCacheSeconds;
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

SidebarWidgetDefinition getSidebarWidgetByType(SidebarWidgetType type) {
  switch (type) {
  case BATTERY_METER:
    return (SidebarWidgetDefinition){BATTERY_METER, SIDEBAR_CONTENT_CUSTOM,
                                     NULL, BatteryMeter_measure,
                                     BatteryMeter_draw};
  case BLUETOOTH_DISCONNECT:
    return (SidebarWidgetDefinition){BLUETOOTH_DISCONNECT, SIDEBAR_CONTENT_ICON,
                                     BTDisconnect_getContent, NULL, NULL};
  case DATE:
    return (SidebarWidgetDefinition){DATE, SIDEBAR_CONTENT_DATE,
                                     Date_getContent, NULL, NULL};
  case WORLD_TIME_1:
    return (SidebarWidgetDefinition){WORLD_TIME_1, SIDEBAR_CONTENT_LABEL_VALUE,
                                     WorldTime1_getContent, NULL, NULL};
  case WORLD_TIME_2:
    return (SidebarWidgetDefinition){WORLD_TIME_2, SIDEBAR_CONTENT_LABEL_VALUE,
                                     WorldTime2_getContent, NULL, NULL};
  case NEXT_SUN_EVENT:
    return (SidebarWidgetDefinition){NEXT_SUN_EVENT, SIDEBAR_CONTENT_ICON_VALUE,
                                     NextSunEvent_getContent, NULL, NULL};
  case SECONDS:
    return (SidebarWidgetDefinition){SECONDS, SIDEBAR_CONTENT_TEXT,
                                     Seconds_getContent, NULL, NULL};
  case WEATHER_CURRENT:
    return (SidebarWidgetDefinition){WEATHER_CURRENT,
                                     SIDEBAR_CONTENT_ICON_VALUE,
                                     CurrentWeather_getContent, NULL, NULL};
  case WEATHER_FORECAST_TODAY:
    return (SidebarWidgetDefinition){WEATHER_FORECAST_TODAY,
                                     SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED,
                                     WeatherForecast_getContent, NULL, NULL};
  case WEEK_NUMBER:
    return (SidebarWidgetDefinition){WEEK_NUMBER, SIDEBAR_CONTENT_LABEL_VALUE,
                                     WeekNumber_getContent, NULL, NULL};
  case WEATHER_UV_INDEX:
    return (SidebarWidgetDefinition){WEATHER_UV_INDEX,
                                     SIDEBAR_CONTENT_LABEL_VALUE,
                                     UVIndex_getContent, NULL, NULL};
  case WEATHER_WIND:
    return (SidebarWidgetDefinition){WEATHER_WIND, SIDEBAR_CONTENT_LABEL_VALUE,
                                     Wind_getContent, NULL, NULL};
  case WEATHER_PRECIP_CHANCE:
    return (SidebarWidgetDefinition){WEATHER_PRECIP_CHANCE,
                                     SIDEBAR_CONTENT_LABEL_VALUE,
                                     PrecipChance_getContent, NULL, NULL};
#ifdef PBL_HEALTH
  case STEP_COUNTER:
    return (SidebarWidgetDefinition){STEP_COUNTER, SIDEBAR_CONTENT_ICON_VALUE,
                                     StepCounter_getContent, NULL, NULL};
  case SLEEP_TIMER:
    return (SidebarWidgetDefinition){SLEEP_TIMER,
                                     SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT,
                                     SleepTimer_getContent, NULL, NULL};
  case HEARTRATE:
    return (SidebarWidgetDefinition){HEARTRATE, SIDEBAR_CONTENT_ICON_VALUE,
                                     HeartRate_getContent, NULL, NULL};
#endif
  case BEATS:
    return (SidebarWidgetDefinition){BEATS, SIDEBAR_CONTENT_LABEL_VALUE,
                                     Beats_getContent, NULL, NULL};
  default:
    return (SidebarWidgetDefinition){EMPTY, SIDEBAR_CONTENT_EMPTY,
                                     Empty_getContent, NULL, NULL};
  }
}
