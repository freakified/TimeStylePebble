#include "sidebar_widgets.h"
#include "languages.h"
#include "settings.h"
#include "text_metrics.h"
#include "util.h"
#include "weather.h"
#include <math.h>
#include <pebble.h>

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

#define DATE_CALENDAR_DAY_TOP_PADDING 10
#define DATE_CALENDAR_DAY_BOTTOM_PADDING 4
#define DATE_CALENDAR_DAY_TOP_PADDING_LARGE 6
#define DATE_CALENDAR_DAY_BOTTOM_PADDING_LARGE 2

GDrawCommandImage *dateImage;
GDrawCommandImage *disconnectImage;
GDrawCommandImage *batteryImage;
GDrawCommandImage *batteryChargeImage;
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
GDrawCommandImage *dateLgImage;
#endif

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
char altClock[8];
char currentBeats[5];

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
static SidebarContent AltTime_getContent();
static SidebarContent Beats_getContent();
static SidebarContent UVIndex_getContent();

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
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  dateLgImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG_LG);
#endif
  disconnectImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
  batteryImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
  batteryChargeImage =
      gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

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
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  gdraw_command_image_destroy(dateLgImage);
#endif
  gdraw_command_image_destroy(disconnectImage);
  gdraw_command_image_destroy(batteryImage);
  gdraw_command_image_destroy(batteryChargeImage);

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

  if (dynamicSettings.enableAltTimeZone) {
    int hour =
        timeInfo->tm_hour - timeInfo->tm_gmtoff / 60 / 60 - timeInfo->tm_isdst;
    hour += settings.altclockOffset;

    char am_pm = (mod(hour, 24) < 12) ? 'a' : 'p';

    if (clock_is_24h_style()) {
      hour = mod(hour, 24);
      am_pm = (char)0;
    } else {
      hour = mod(hour, 12);
      if (hour == 0) {
        hour = 12;
      }
    }

    if (settings.showLeadingZero && hour < 10) {
      snprintf(altClock, sizeof(altClock), "0%i%c", hour, am_pm);
    } else {
      snprintf(altClock, sizeof(altClock), "%i%c", hour, am_pm);
    }
  }

  if (dynamicSettings.enableBeats) {
    snprintf(currentBeats, sizeof(currentBeats), "%i",
             time_get_beats(timeInfo));
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

static int render_width(SidebarRenderSpec *spec) {
  return spec->contentWidth + SIDEBAR_TEXT_OVERFLOW * 2;
}

static int text_x(GRect frame, int width) {
  return frame.origin.x + (frame.size.w - width) / 2;
}

static int icon_x(GRect frame) {
  return frame.origin.x + (frame.size.w - SIDEBAR_ICON_SIZE) / 2;
}

static int icon_text_gap(SidebarRenderSpec *spec) {
  return spec->compact ? 5 : SIDEBAR_ICON_TEXT_GAP;
}

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
                                     int visibleTop, int width,
                                     GFont primaryFont, GFont fallbackFont,
                                     SidebarTextMetrics primaryMetrics,
                                     SidebarTextMetrics fallbackMetrics) {
  GRect fitRect = GRect(x, visibleTop, width, primaryMetrics.height);
  GFont selectedFont = primaryFont;
  const char *text = pick_text(candidates, primaryFont, fallbackFont,
                               fit_rect(fitRect), &selectedFont);
  SidebarTextMetrics selectedMetrics =
      (selectedFont == primaryFont) ? primaryMetrics : fallbackMetrics;
  GRect drawRect =
      text_rect_at_visible_top(x, visibleTop, width, selectedMetrics);

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  graphics_draw_text(ctx, text, selectedFont, drawRect, GTextOverflowModeFill,
                     GTextAlignmentCenter, NULL);
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

  gdraw_command_image_recolor(content->icon, dynamicSettings.iconFillColor,
                              dynamicSettings.iconStrokeColor);
  gdraw_command_image_draw(ctx, content->icon,
                           GPoint(iconFrame.origin.x, iconFrame.origin.y));
}

static void draw_battery_icon(GContext *ctx, GRect iconFrame,
                              uint8_t batteryPercent, bool batteryCharging) {
  if (!batteryImage) {
    return;
  }

  int batteryY = iconFrame.origin.y - 5;
  gdraw_command_image_recolor(batteryImage, dynamicSettings.iconFillColor,
                              dynamicSettings.iconStrokeColor);
  gdraw_command_image_draw(ctx, batteryImage,
                           GPoint(iconFrame.origin.x, batteryY));

  if (batteryCharging) {
    if (batteryChargeImage) {
      gdraw_command_image_recolor(batteryChargeImage,
                                  dynamicSettings.iconStrokeColor,
                                  dynamicSettings.iconFillColor);
      gdraw_command_image_draw(ctx, batteryChargeImage,
                               GPoint(iconFrame.origin.x, batteryY));
    }
    return;
  }

  int width = roundf(18 * batteryPercent / 100.0f);
  graphics_context_set_fill_color(ctx, dynamicSettings.iconStrokeColor);

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

static SidebarMeasure generic_measure(SidebarContent *content,
                                      SidebarRenderSpec *spec) {
  int textHeight = single_text_height();
  int height = 0;

  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    height = 0;
    break;
  case SIDEBAR_CONTENT_TEXT:
    height = large_text_height();
    break;
  case SIDEBAR_CONTENT_ICON:
    height = SIDEBAR_ICON_SIZE;
    break;
  case SIDEBAR_CONTENT_ICON_VALUE:
    if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
      height = SIDEBAR_ICON_SIZE > textHeight ? SIDEBAR_ICON_SIZE : textHeight;
    } else {
      height = SIDEBAR_ICON_SIZE + icon_text_gap(spec) + textHeight;
    }
    break;
  case SIDEBAR_CONTENT_LABEL_VALUE:
    height = basic_padding_top(spec) + small_text_height() +
             basic_label_gap(spec) + textHeight + basic_padding_bottom(spec);
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED:
    if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
      int pairHeight = single_text_height() + stacked_value_gap(spec) +
                       SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
                       stacked_value_gap(spec) + single_text_height();
      height = SIDEBAR_ICON_SIZE > pairHeight ? SIDEBAR_ICON_SIZE : pairHeight;
    } else {
      height = SIDEBAR_ICON_SIZE + icon_text_gap(spec) + single_text_height() +
               stacked_value_gap(spec) + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
               stacked_value_gap(spec) + single_text_height();
    }
    break;
  case SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT:
    if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
      int stackHeight =
          medium_text_height() + stacked_value_gap(spec) + small_text_height();
      height =
          SIDEBAR_ICON_SIZE > stackHeight ? SIDEBAR_ICON_SIZE : stackHeight;
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

  return (SidebarMeasure){spec->contentWidth, height};
}

static void draw_loading_text(GContext *ctx, int x, int visibleTop,
                              int width, GFont font,
                              SidebarTextMetrics metrics) {
  draw_literal_text_at_visible_top(ctx, "...", x, visibleTop, width, font,
                                   metrics, settings.sidebarTextColor);
}

static void draw_icon_value(GContext *ctx, SidebarContent *content, GRect frame,
                            SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos;
  int textTop;
  GRect iconFrame;

  if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
    iconFrame =
        GRect(frame.origin.x,
              frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
              SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + SIDEBAR_ICON_SIZE + 3;
    textWidth = frame.size.w - SIDEBAR_ICON_SIZE - 3;
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

  draw_text_at_visible_top(ctx, &content->primary, textXPos, textTop, textWidth,
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

static void draw_label_value(GContext *ctx, SidebarContent *content,
                             GRect frame, SidebarRenderSpec *spec) {
  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int labelTop = frame.origin.y + basic_padding_top(spec);
  int valueTop = labelTop + fontMetrics.sm.height + basic_label_gap(spec);

  draw_literal_text_at_visible_top(ctx, content->label, textXPos, labelTop,
                                   textWidth, smSidebarFont, fontMetrics.sm,
                                   settings.sidebarTextColor);
  draw_text_at_visible_top(ctx, &content->primary, textXPos, valueTop,
                           textWidth, currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
}

static void get_icon_stack_layout(GRect frame, SidebarRenderSpec *spec,
                                  int stackHeight, GRect *iconFrame, int *textX,
                                  int *textWidth, int *textTop) {
  int renderTextWidth = render_width(spec);

  if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
    *iconFrame = GRect(frame.origin.x,
                       frame.origin.y + (frame.size.h - SIDEBAR_ICON_SIZE) / 2,
                       SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    *textTop = frame.origin.y + (frame.size.h - stackHeight) / 2;
    *textX = frame.origin.x + SIDEBAR_ICON_SIZE + 3;
    *textWidth = frame.size.w - SIDEBAR_ICON_SIZE - 3;
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

  get_icon_stack_layout(frame, spec, pairHeight, &iconFrame, &textXPos,
                        &textWidth, &textTop);
  draw_icon(ctx, content, iconFrame);

  if (content->isLoading) {
    draw_loading_text(ctx, textXPos, textTop, textWidth, currentSidebarFont,
                      fontMetrics.current);
    return;
  }

  draw_text_at_visible_top(ctx, &content->primary, textXPos, textTop, textWidth,
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());

  dividerY = textTop + fontMetrics.current.height + stacked_value_gap(spec);
  dividerWidth = spec->contentWidth - SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN * 2;
  if (dividerWidth < 1) {
    dividerWidth = 1;
  }
  dividerX = frame.origin.x + (frame.size.w - spec->contentWidth) / 2 +
             SIDEBAR_VALUE_PAIR_DIVIDER_MARGIN;

  graphics_context_set_fill_color(ctx, settings.sidebarTextColor);
  graphics_fill_rect(ctx,
                     GRect(dividerX, dividerY, dividerWidth,
                           SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT),
                     0, GCornerNone);

  draw_text_at_visible_top(ctx, &content->secondary, textXPos,
                           dividerY + SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT +
                               stacked_value_gap(spec),
                           textWidth, currentSidebarFont, fallbackSidebarFont,
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
                           primaryWidth, mdSidebarFont, fallbackSidebarFont,
                           fontMetrics.md, fallback_current_metrics());
  draw_text_at_visible_top(
      ctx, &content->secondary, primaryX,
      textTop + fontMetrics.md.height + stacked_value_gap(spec), primaryWidth,
      smSidebarFont, smSidebarFont, fontMetrics.sm, fontMetrics.sm);
}

static void generic_draw(GContext *ctx, SidebarContent *content, GRect frame,
                         SidebarRenderSpec *spec) {
  switch (content->kind) {
  case SIDEBAR_CONTENT_EMPTY:
    return;
  case SIDEBAR_CONTENT_TEXT: {
    int width = render_width(spec);
    draw_text_at_visible_top(ctx, &content->primary, text_x(frame, width),
                             frame.origin.y, width, lgSidebarFont,
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
    return (SidebarMeasure){spec->contentWidth, 14};
  }

  if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
    int textHeight = single_text_height();
    return (SidebarMeasure){spec->contentWidth,
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

  if (spec->mode == SIDEBAR_LAYOUT_HORIZONTAL) {
    iconFrame = GRect(frame.origin.x, frame.origin.y + (frame.size.h - 14) / 2,
                      SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    textXPos = frame.origin.x + SIDEBAR_ICON_SIZE + 3;
    textWidth = frame.size.w - SIDEBAR_ICON_SIZE - 3;
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
  draw_text_at_visible_top(ctx, &batteryText, textXPos, textTop, textWidth,
                           currentSidebarFont, fallbackSidebarFont,
                           fontMetrics.current, fallback_current_metrics());
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
  return 30;
#else
  return settings.useLargeFonts ? 26 : SIDEBAR_ICON_SIZE;
#endif
}

static DateLayout date_layout(SidebarRenderSpec *spec) {
  int weekdayToCalendarGap = icon_text_gap(spec);
  int calendarToMonthGap = icon_text_gap(spec);
  int calendarTop = fontMetrics.current.height + weekdayToCalendarGap;
  int calendarHeight = date_day_top_padding() + fontMetrics.current.height +
                       date_day_bottom_padding();
  int monthTop = calendarTop + calendarHeight + calendarToMonthGap;

  return (DateLayout){
      .calendarTop = calendarTop,
      .calendarHeight = calendarHeight,
      .calendarWidth = date_calendar_width(),
      .dayNumTop = calendarTop + date_day_top_padding(),
      .monthTop = monthTop,
      .height = monthTop + fontMetrics.current.height,
      .compactHeight = calendarTop + calendarHeight,
  };
}

static SidebarMeasure date_content_measure(SidebarRenderSpec *spec) {
  DateLayout metrics = date_layout(spec);
  return (SidebarMeasure){spec->contentWidth, spec->compact
                                                  ? metrics.compactHeight
                                                  : metrics.height};
}

static void date_content_draw(GContext *ctx, GRect frame,
                              SidebarRenderSpec *spec) {
  DateLayout metrics = date_layout(spec);
  int textWidth = render_width(spec);
  int textXPos = text_x(frame, textWidth);
  int calendarX = frame.origin.x + (frame.size.w - metrics.calendarWidth) / 2;
  int calendarY = frame.origin.y + metrics.calendarTop;

  draw_literal_text_at_visible_top(
      ctx, currentDayName, textXPos, frame.origin.y, textWidth,
      currentSidebarFont, fontMetrics.current, settings.sidebarTextColor);

  if (!settings.useLargeFonts) {
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
    GDrawCommandImage *activeDateImage = dateLgImage;
#else
    GDrawCommandImage *activeDateImage = dateImage;
#endif
    if (activeDateImage) {
      gdraw_command_image_recolor(activeDateImage,
                                  dynamicSettings.iconFillColor,
                                  dynamicSettings.iconStrokeColor);
      gdraw_command_image_draw(ctx, activeDateImage,
                               GPoint(calendarX, calendarY));
    }
  } else {
    graphics_context_set_fill_color(ctx, dynamicSettings.iconStrokeColor);
    graphics_fill_rect(ctx,
                       GRect(calendarX, calendarY, metrics.calendarWidth,
                             metrics.calendarHeight),
                       2, GCornersAll);

    graphics_context_set_fill_color(ctx, dynamicSettings.iconFillColor);
    graphics_fill_rect(ctx,
                       GRect(calendarX + 2, calendarY + 2,
                             metrics.calendarWidth - 4,
                             metrics.calendarHeight - 4),
                       0, GCornersAll);
  }

  draw_literal_text_at_visible_top(
      ctx, currentDayNum, textXPos, frame.origin.y + metrics.dayNumTop,
      textWidth, currentSidebarFont, fontMetrics.current,
      dynamicSettings.iconStrokeColor);

  if (!spec->compact) {
    draw_literal_text_at_visible_top(
        ctx, currentMonth, textXPos, frame.origin.y + metrics.monthTop,
        textWidth, currentSidebarFont, fontMetrics.current,
        settings.sidebarTextColor);
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

static SidebarContent AltTime_getContent() {
  SidebarContent content = empty_content(SIDEBAR_CONTENT_LABEL_VALUE);
  content.label = settings.altclockName;
  content.primary = same_text_candidates(altClock);
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
  int sleepSeconds = is_health_metric_accessible(metric)
                         ? (int)health_service_sum_today(metric)
                         : 0;
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
  case ALT_TIME_ZONE:
    return (SidebarWidgetDefinition){ALT_TIME_ZONE, SIDEBAR_CONTENT_LABEL_VALUE,
                                     AltTime_getContent, NULL, NULL};
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
