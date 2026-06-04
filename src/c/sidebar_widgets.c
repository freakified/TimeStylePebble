#include "sidebar_widgets.h"
#include "languages.h"
#include "settings.h"
#include "text_metrics.h"
#include "util.h"
#include "weather.h"
#include <math.h>
#include <pebble.h>

bool SidebarWidgets_useCompactMode = false;
int SidebarWidgets_xOffset;
int SidebarWidgets_contentWidth = 30;

#define STANDARD_WIDGET_TEXT_MARGIN 2
#define STANDARD_WIDGET_TEXT_OVERFLOW 5

// sidebar icons
GDrawCommandImage *dateImage;
GDrawCommandImage *disconnectImage;
GDrawCommandImage *batteryImage;
GDrawCommandImage *batteryChargeImage;
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
GDrawCommandImage *dateLgImage;
#endif

// fonts
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
  // battery meter
  int batteryGraphicOnlyHeight;
  int batteryWithPctHeight;
  int batteryTextY;
  // date
  int dateHeight, dateHeightCompact;
  int dateTopCorrection, dateDayNumY, dateMonthY;
  int dateBgX, dateBgY;
  int dateBgRectX, dateBgRectY, dateBgRectWidth, dateBgRectHeight;
  // current weather
  int weatherHeight;
  int weatherTempY;
  // common text rect x offset and width for main data display
  int textRectX;
  int textRectWidth;
  int textRectHeight;
  // weather forecast
  int weatherForecastHeight;
  int forecastHighY, forecastDividerY, forecastLowY;
  int forecastDividerX, forecastDividerWidth;
  // basic widgets (week number, alt time, UV index, beats)
  int basicWidgetHeight;
  int basicWidgetLabelY;
  int basicWidgetY;
  // bluetooth disconnect
  int btDisconnectHeight;
  // heart rate
  int heartRateHeight;
  int heartRateValueY;
  // steps
  int stepCounterHeight;
  int stepsTextY;
  // sleep
  int sleepTimerHeight;
  int sleepHoursY, sleepMinutesY;
  // seconds
  int secondsHeight, secondsY;
} SidebarWidgetLayout;

static SidebarWidgetLayout layout;
static SidebarFontMetrics fontMetrics;

typedef struct {
  GDrawCommandImage *icon;
  char full[16];
  char compact[16];
  char tiny[16];
  bool available;
} StandardWidgetData;

typedef struct {
  GRect fitRect;
  GRect drawRect;
  int iconX;
  int iconY;
  GFont primaryFont;
  GFont fallbackFont;
} StandardWidgetRenderContext;

typedef struct {
  const char *label;
  StandardWidgetData value;
} BasicWidgetData;

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

static int text_draw_y_for_baseline(SidebarTextMetrics metrics, int baselineY) {
  return baselineY - metrics.offset - metrics.height;
}

static int text_rect_height(SidebarTextMetrics metrics) {
  return metrics.offset + metrics.height;
}

// the date, time and weather strings
char currentDayName[8];
char currentDayNum[5];
char currentMonth[8];
char currentWeekNum[5];
char currentSecondsNum[5];
char altClock[8];
char currentBeats[5];

// the widgets
SidebarWidget batteryMeterWidget;
int BatteryMeter_getHeight();
void BatteryMeter_draw(GContext *ctx, int yPosition);

SidebarWidget emptyWidget;
int EmptyWidget_getHeight();
void EmptyWidget_draw(GContext *ctx, int yPosition);

SidebarWidget dateWidget;
int DateWidget_getHeight();
void DateWidget_draw(GContext *ctx, int yPosition);

SidebarWidget currentWeatherWidget;
int CurrentWeather_getHeight();
void CurrentWeather_draw(GContext *ctx, int yPosition);

SidebarWidget weatherForecastWidget;
int WeatherForecast_getHeight();
void WeatherForecast_draw(GContext *ctx, int yPosition);

SidebarWidget btDisconnectWidget;
int BTDisconnect_getHeight();
void BTDisconnect_draw(GContext *ctx, int yPosition);

SidebarWidget weekNumberWidget;
int WeekNumber_getHeight();
void WeekNumber_draw(GContext *ctx, int yPosition);

SidebarWidget secondsWidget;
int Seconds_getHeight();
void Seconds_draw(GContext *ctx, int yPosition);

SidebarWidget altTimeWidget;
int AltTime_getHeight();
void AltTime_draw(GContext *ctx, int yPosition);

SidebarWidget beatsWidget;
int Beats_getHeight();
void Beats_draw(GContext *ctx, int yPosition);

SidebarWidget uvIndexWidget;
int UVIndex_getHeight();
void UVIndex_draw(GContext *ctx, int yPosition);

#ifdef PBL_HEALTH
GDrawCommandImage *sleepImage;
GDrawCommandImage *stepsImage;
GDrawCommandImage *heartImage;

SidebarWidget stepCounterWidget;
int StepCounter_getHeight();
void StepCounter_draw(GContext *ctx, int yPosition);

SidebarWidget sleepTimerWidget;
int SleepTimer_getHeight();
void SleepTimer_draw(GContext *ctx, int yPosition);

SidebarWidget heartRateWidget;
int HeartRate_getHeight();
void HeartRate_draw(GContext *ctx, int yPosition);
#endif

void SidebarWidgets_init() {
// load fonts
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
#else
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
#endif

  // load the sidebar graphics
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

  // set up widgets' function pointers correctly
  batteryMeterWidget.getHeight = BatteryMeter_getHeight;
  batteryMeterWidget.draw = BatteryMeter_draw;

  emptyWidget.getHeight = EmptyWidget_getHeight;
  emptyWidget.draw = EmptyWidget_draw;

  dateWidget.getHeight = DateWidget_getHeight;
  dateWidget.draw = DateWidget_draw;

  currentWeatherWidget.getHeight = CurrentWeather_getHeight;
  currentWeatherWidget.draw = CurrentWeather_draw;

  weatherForecastWidget.getHeight = WeatherForecast_getHeight;
  weatherForecastWidget.draw = WeatherForecast_draw;

  btDisconnectWidget.getHeight = BTDisconnect_getHeight;
  btDisconnectWidget.draw = BTDisconnect_draw;

  weekNumberWidget.getHeight = WeekNumber_getHeight;
  weekNumberWidget.draw = WeekNumber_draw;

  secondsWidget.getHeight = Seconds_getHeight;
  secondsWidget.draw = Seconds_draw;

  altTimeWidget.getHeight = AltTime_getHeight;
  altTimeWidget.draw = AltTime_draw;

  uvIndexWidget.getHeight = UVIndex_getHeight;
  uvIndexWidget.draw = UVIndex_draw;

#ifdef PBL_HEALTH
  stepCounterWidget.getHeight = StepCounter_getHeight;
  stepCounterWidget.draw = StepCounter_draw;

  sleepTimerWidget.getHeight = SleepTimer_getHeight;
  sleepTimerWidget.draw = SleepTimer_draw;

  heartRateWidget.getHeight = HeartRate_getHeight;
  heartRateWidget.draw = HeartRate_draw;
#endif

  beatsWidget.getHeight = Beats_getHeight;
  beatsWidget.draw = Beats_draw;
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

  const int batteryIconHeight = 14;
  const int batteryTextTop = 20;
  const int iconHeight = 22;
  const int iconTextGap = 9;
  const int stackedValueGap = 7;
  const int basicWidgetPaddingTop = 2;
  const int basicWidgetGap = 6;
  const int basicWidgetPaddingBottom = 5;
  const int dateDayTop = 26;
  const int dateMonthTop = 47;
  const int dateImageDayInset = 10;
  const int dateRectDayInset = 6;
  const int forecastDividerGap = 5;
  const int secondsBaseline = 18;

  int iconTextTop = iconHeight + iconTextGap;
  int basicValueTop =
      basicWidgetPaddingTop + fontMetrics.sm.height + basicWidgetGap;
  int forecastLowTop =
      iconTextTop + fontMetrics.current.height + stackedValueGap;
  int forecastDividerTop =
      iconTextTop + fontMetrics.current.height + forecastDividerGap;

  layout = (SidebarWidgetLayout){
      .batteryGraphicOnlyHeight = batteryIconHeight,
      .batteryWithPctHeight = batteryTextTop + fontMetrics.current.height,
      .batteryTextY = text_draw_y(fontMetrics.current, batteryTextTop) + 5,
      .dateHeight = dateMonthTop + fontMetrics.current.height,
      .dateHeightCompact = dateDayTop + fontMetrics.current.height,
      .dateTopCorrection = fontMetrics.current.offset,
      .dateDayNumY = dateDayTop,
      .dateMonthY = dateMonthTop,
      .dateBgX = 3,
      .dateBgY =
          dateDayTop - dateImageDayInset + fontMetrics.current.offset,
      .dateBgRectX = 2,
      .dateBgRectY =
          dateDayTop - dateRectDayInset + fontMetrics.current.offset,
      .dateBgRectWidth = 26,
      .dateBgRectHeight = 22,
      .weatherHeight = iconTextTop + fontMetrics.current.height,
      .weatherTempY = text_draw_y(fontMetrics.current, iconTextTop),
      .textRectX = -5,
      .textRectWidth = 40,
      .textRectHeight = text_rect_height(fontMetrics.current),
      .weatherForecastHeight =
          forecastLowTop + fontMetrics.current.height + basicWidgetPaddingBottom,
      .forecastHighY = text_draw_y(fontMetrics.current, iconTextTop),
      .forecastDividerY = forecastDividerTop - 8,
      .forecastLowY = text_draw_y(fontMetrics.current, forecastLowTop),
      .forecastDividerX = 3,
      .forecastDividerWidth = 24,
      .basicWidgetHeight =
          basicValueTop + fontMetrics.current.height + basicWidgetPaddingBottom,
      .basicWidgetLabelY = text_draw_y(fontMetrics.sm, basicWidgetPaddingTop),
      .basicWidgetY = text_draw_y(fontMetrics.current, basicValueTop),
      .btDisconnectHeight = iconHeight,
      .heartRateHeight = iconTextTop + fontMetrics.current.height,
      .heartRateValueY = text_draw_y(fontMetrics.current, iconTextTop),
      .stepCounterHeight = iconTextTop + fontMetrics.current.height,
      .stepsTextY = text_draw_y(fontMetrics.current, iconTextTop),
      .sleepTimerHeight = iconTextTop + fontMetrics.md.height +
                          stackedValueGap + fontMetrics.sm.height,
      .sleepHoursY = text_draw_y(fontMetrics.md, iconTextTop),
      .sleepMinutesY = text_draw_y(
          fontMetrics.sm,
          iconTextTop + fontMetrics.md.height + stackedValueGap),
      .secondsHeight = fontMetrics.lg.height,
      .secondsY = text_draw_y_for_baseline(fontMetrics.lg, secondsBaseline),
  };

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  layout.dateBgX = 1;
  layout.dateBgRectX = 0;
  layout.dateBgRectWidth = 30;
  layout.dateBgRectHeight = 26;
  layout.textRectX = -10;
  layout.textRectWidth = 50;
  layout.forecastDividerX = 1;
  layout.forecastDividerWidth = 28;
#endif
}

// c can't do true modulus on negative numbers, apparently
// from
// http://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

void SidebarWidgets_updateTime(struct tm *timeInfo) {
  // printf("Current RAM: %d", heap_bytes_free());

  // set all the date strings
  strftime(currentDayNum, 3, "%e", timeInfo);
  // remove padding on date num, if needed
  if (currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }

  strftime(currentWeekNum, 3, "%V", timeInfo);

  strncpy(currentDayName, dayNames[settings.languageId][timeInfo->tm_wday],
          sizeof(currentDayName));
  strncpy(currentMonth, monthNames[settings.languageId][timeInfo->tm_mon],
          sizeof(currentMonth));

  // set the seconds string
  strftime(currentSecondsNum, 4, ":%S", timeInfo);

  if (dynamicSettings.enableAltTimeZone) {
    // set the alternate time zone string
    int hour =
        timeInfo->tm_hour - timeInfo->tm_gmtoff / 60 / 60 - timeInfo->tm_isdst;

    // apply the configured offset value
    hour += settings.altclockOffset;

    char am_pm = (mod(hour, 24) < 12) ? 'a' : 'p';

    // format it
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
    int beats = 0;

    // set the swatch internet time beats
    beats = time_get_beats(timeInfo);

    snprintf(currentBeats, sizeof(currentBeats), "%i", beats);
  }
}

/* Sidebar Widget Selection */
SidebarWidget getSidebarWidgetByType(SidebarWidgetType type) {
  switch (type) {
  case BATTERY_METER:
    return batteryMeterWidget;
    break;
  case BLUETOOTH_DISCONNECT:
    return btDisconnectWidget;
    break;
  case DATE:
    return dateWidget;
    break;
  case ALT_TIME_ZONE:
    return altTimeWidget;
    break;
  case SECONDS:
    return secondsWidget;
    break;
  case WEATHER_CURRENT:
    return currentWeatherWidget;
    break;
  case WEATHER_FORECAST_TODAY:
    return weatherForecastWidget;
    break;
  case WEEK_NUMBER:
    return weekNumberWidget;
  case WEATHER_UV_INDEX:
    return uvIndexWidget;
#ifdef PBL_HEALTH
  case STEP_COUNTER:
    return stepCounterWidget;
  case SLEEP_TIMER:
    return sleepTimerWidget;
  case HEARTRATE:
    return heartRateWidget;
#endif
  case BEATS:
    return beatsWidget;
  default:
    return emptyWidget;
    break;
  }
}

static bool text_fits(const char *text, GFont font, GRect rect) {
  GRect measureRect = GRect(0, 0, 200, rect.size.h);
  GSize size = graphics_text_layout_get_content_size(
      text, font, measureRect, GTextOverflowModeFill, GTextAlignmentCenter);

  return size.w <= rect.size.w;
}

static const char *pick_standard_widget_text(StandardWidgetData *data,
                                             StandardWidgetRenderContext *ctx,
                                             GFont *selectedFont) {
  GFont fonts[2] = {ctx->primaryFont, ctx->fallbackFont};

  for (int fontIndex = 0; fontIndex < 2; fontIndex++) {
    if (text_fits(data->full, fonts[fontIndex], ctx->fitRect)) {
      *selectedFont = fonts[fontIndex];
      return data->full;
    }

    if (text_fits(data->compact, fonts[fontIndex], ctx->fitRect)) {
      *selectedFont = fonts[fontIndex];
      return data->compact;
    }
  }

  *selectedFont = ctx->fallbackFont;
  return data->tiny;
}

static void draw_standard_widget(GContext *ctx, StandardWidgetData *data,
                                 StandardWidgetRenderContext renderCtx) {
  if (data->icon) {
    gdraw_command_image_recolor(data->icon, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, data->icon,
                             GPoint(renderCtx.iconX, renderCtx.iconY));
  }

  if (!data->available) {
    graphics_context_set_text_color(ctx, settings.sidebarTextColor);
    graphics_draw_text(ctx, "...", renderCtx.fallbackFont, renderCtx.drawRect,
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    return;
  }

  GFont selectedFont = renderCtx.primaryFont;
  const char *text = pick_standard_widget_text(data, &renderCtx, &selectedFont);

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  graphics_draw_text(ctx, text, selectedFont, renderCtx.drawRect,
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static int get_standard_widget_text_x(int textWidth) {
  return SidebarWidgets_xOffset - (textWidth - 30) / 2;
}

static int get_standard_widget_text_width() {
  return SidebarWidgets_contentWidth;
}

static int get_standard_widget_draw_width() {
  return SidebarWidgets_contentWidth + STANDARD_WIDGET_TEXT_OVERFLOW * 2;
}

static GRect get_standard_widget_fit_rect(int x, int y, int width) {
  int margin = width > STANDARD_WIDGET_TEXT_MARGIN * 2
                   ? STANDARD_WIDGET_TEXT_MARGIN
                   : 0;
  return GRect(x + margin, y, width - margin * 2, layout.textRectHeight);
}

static GRect get_standard_widget_draw_rect(int y) {
  int width = get_standard_widget_draw_width();
  int x = get_standard_widget_text_x(width);
  return GRect(x, y, width, layout.textRectHeight);
}

static StandardWidgetData get_standard_text_data(const char *text) {
  StandardWidgetData data = (StandardWidgetData){
      .available = true,
  };

  snprintf(data.full, sizeof(data.full), "%s", text);
  snprintf(data.compact, sizeof(data.compact), "%s", text);
  snprintf(data.tiny, sizeof(data.tiny), "%s", text);

  return data;
}

static void draw_basic_standard_widget(GContext *ctx, BasicWidgetData *data,
                                       int yPosition, int valueYOffset) {
  int textWidth = get_standard_widget_text_width();
  int textX = get_standard_widget_text_x(textWidth);
  GRect labelRect = get_standard_widget_draw_rect(
      yPosition + layout.basicWidgetLabelY);

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  graphics_draw_text(ctx, data->label, smSidebarFont,
                     labelRect, GTextOverflowModeFill, GTextAlignmentCenter,
                     NULL);

  StandardWidgetRenderContext renderCtx = (StandardWidgetRenderContext){
      .fitRect = get_standard_widget_fit_rect(textX, yPosition + valueYOffset,
                                              textWidth),
      .drawRect = get_standard_widget_draw_rect(yPosition + valueYOffset),
      .primaryFont = currentSidebarFont,
      .fallbackFont = fallbackSidebarFont,
  };

  draw_standard_widget(ctx, &data->value, renderCtx);
}

/********** functions for the empty widget **********/
int EmptyWidget_getHeight() { return 0; }

void EmptyWidget_draw(GContext *ctx, int yPosition) { return; }

/********** functions for the battery meter widget **********/

int BatteryMeter_getHeight() {
  BatteryChargeState chargeState = battery_state_service_peek();

  if (chargeState.is_charging || !settings.showBatteryPct) {
    return layout.batteryGraphicOnlyHeight;
  } else {
    return layout.batteryWithPctHeight;
  }
}

static StandardWidgetData BatteryMeter_getStandardData(uint8_t batteryPercent) {
  StandardWidgetData data = (StandardWidgetData){
      .available = true,
  };

  if (settings.languageId == LANGUAGE_TR) {
    snprintf(data.full, sizeof(data.full), "%%%d", batteryPercent);
  } else {
    snprintf(data.full, sizeof(data.full), "%d%%", batteryPercent);
  }

  snprintf(data.compact, sizeof(data.compact), "%d", batteryPercent);
  snprintf(data.tiny, sizeof(data.tiny), "%d", batteryPercent);

  return data;
}

void BatteryMeter_draw(GContext *ctx, int yPosition) {

  BatteryChargeState chargeState = battery_state_service_peek();
  uint8_t battery_percent =
      (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  int batteryPositionY =
      yPosition - 5; // correct for vertical empty space on battery icon

  if (batteryImage) {
    gdraw_command_image_recolor(batteryImage, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(
        ctx, batteryImage,
        GPoint(3 + SidebarWidgets_xOffset, batteryPositionY));
  }

  if (chargeState.is_charging) {
    if (batteryChargeImage) {
      // the charge "bolt" icon uses inverted colors
      gdraw_command_image_recolor(batteryChargeImage,
                                  dynamicSettings.iconStrokeColor,
                                  dynamicSettings.iconFillColor);
      gdraw_command_image_draw(
          ctx, batteryChargeImage,
          GPoint(3 + SidebarWidgets_xOffset, batteryPositionY));
    }
  } else {

    int width = roundf(18 * battery_percent / 100.0f);

    graphics_context_set_fill_color(ctx, dynamicSettings.iconStrokeColor);

#ifdef PBL_COLOR
    if (battery_percent <= 20) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
#endif

    graphics_fill_rect(
        ctx, GRect(6 + SidebarWidgets_xOffset, 8 + batteryPositionY, width, 8),
        0, GCornerNone);
  }

  // never show battery % while charging, because of this issue:
  // https://github.com/freakified/TimeStylePebble/issues/11
  if (settings.showBatteryPct && !chargeState.is_charging) {
    StandardWidgetData data = BatteryMeter_getStandardData(battery_percent);
    int textWidth = get_standard_widget_text_width();
    int textX = get_standard_widget_text_x(textWidth);

    StandardWidgetRenderContext renderCtx = (StandardWidgetRenderContext){
        .fitRect =
            get_standard_widget_fit_rect(textX,
                                         layout.batteryTextY + batteryPositionY,
                                         textWidth),
        .drawRect =
            get_standard_widget_draw_rect(layout.batteryTextY + batteryPositionY),
        .primaryFont = currentSidebarFont,
        .fallbackFont = fallbackSidebarFont,
    };

    draw_standard_widget(ctx, &data, renderCtx);
  }
}

/********** current date widget **********/

int DateWidget_getHeight() {
  return SidebarWidgets_useCompactMode ? layout.dateHeightCompact
                                       : layout.dateHeight;
}

void DateWidget_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  // compensate for extra space that appears on the top of the date widget
  yPosition -= layout.dateTopCorrection;

  // first draw the day name
  graphics_draw_text(ctx, currentDayName, currentSidebarFont,
                     GRect(layout.textRectX + SidebarWidgets_xOffset, yPosition,
                           layout.textRectWidth, layout.textRectHeight),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  // next, draw the date background
  // (an image in normal mode, a rectangle in large font mode)
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
                               GPoint(layout.dateBgX + SidebarWidgets_xOffset,
                                      yPosition + layout.dateBgY));
    }
  } else {
    graphics_context_set_fill_color(ctx, dynamicSettings.iconStrokeColor);
    graphics_fill_rect(ctx,
                       GRect(layout.dateBgRectX + SidebarWidgets_xOffset,
                             yPosition + layout.dateBgRectY,
                             layout.dateBgRectWidth, layout.dateBgRectHeight),
                       2, GCornersAll);

    graphics_context_set_fill_color(ctx, dynamicSettings.iconFillColor);
    graphics_fill_rect(ctx,
                       GRect(layout.dateBgRectX + 2 + SidebarWidgets_xOffset,
                             yPosition + layout.dateBgRectY + 2,
                             layout.dateBgRectWidth - 4,
                             layout.dateBgRectHeight - 4),
                       0, GCornersAll);
  }

  // next, draw the date number
  graphics_context_set_text_color(ctx, dynamicSettings.iconStrokeColor);

  int yOffset = layout.dateDayNumY;

  graphics_draw_text(ctx, currentDayNum, currentSidebarFont,
                     GRect(layout.textRectX + SidebarWidgets_xOffset,
                           yPosition + yOffset, layout.textRectWidth,
                           layout.textRectHeight),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  // switch back to normal color for the rest
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  // don't draw the month if we're in compact mode
  if (!SidebarWidgets_useCompactMode) {
    yOffset = layout.dateMonthY;

    graphics_draw_text(ctx, currentMonth, currentSidebarFont,
                       GRect(layout.textRectX + SidebarWidgets_xOffset,
                             yPosition + yOffset, layout.textRectWidth,
                             layout.textRectHeight),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

/********** current weather widget **********/

int CurrentWeather_getHeight() { return layout.weatherHeight; }

static StandardWidgetData CurrentWeather_getStandardData() {
  StandardWidgetData data = (StandardWidgetData){
      .icon = Weather_currentWeatherIcon,
      .available = Weather_weatherInfo.currentTemp != INT32_MIN,
  };

  if (!data.available) {
    return data;
  }

  int currentTemp = Weather_weatherInfo.currentTemp;

  if (!settings.useMetric) {
    currentTemp = roundf(currentTemp * 1.8f + 32);
  }

  snprintf(data.full, sizeof(data.full), "%d°", currentTemp);
  snprintf(data.compact, sizeof(data.compact), "%d", currentTemp);
  snprintf(data.tiny, sizeof(data.tiny), "%d", currentTemp);
  return data;
}

void CurrentWeather_draw(GContext *ctx, int yPosition) {
  StandardWidgetData data = CurrentWeather_getStandardData();
  int textWidth = get_standard_widget_text_width();
  int textX = get_standard_widget_text_x(textWidth);
  int textY = data.available ? yPosition + layout.weatherTempY : yPosition;

  StandardWidgetRenderContext renderCtx = (StandardWidgetRenderContext){
      .fitRect = get_standard_widget_fit_rect(textX, textY, textWidth),
      .drawRect = get_standard_widget_draw_rect(textY),
      .iconX = 3 + SidebarWidgets_xOffset,
      .iconY = yPosition,
      .primaryFont = currentSidebarFont,
      .fallbackFont = fallbackSidebarFont,
  };

  draw_standard_widget(ctx, &data, renderCtx);
}

/***** Bluetooth Disconnection Widget *****/

int BTDisconnect_getHeight() { return layout.btDisconnectHeight; }

void BTDisconnect_draw(GContext *ctx, int yPosition) {
  if (disconnectImage) {
    gdraw_command_image_recolor(disconnectImage, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);

    gdraw_command_image_draw(ctx, disconnectImage,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }
}

/***** Week Number Widget *****/

int WeekNumber_getHeight() { return layout.basicWidgetHeight; }

static BasicWidgetData WeekNumber_getBasicData() {
  return (BasicWidgetData){
      .label = wordForWeek[settings.languageId],
      .value = get_standard_text_data(currentWeekNum),
  };
}

void WeekNumber_draw(GContext *ctx, int yPosition) {
  BasicWidgetData data = WeekNumber_getBasicData();
  draw_basic_standard_widget(ctx, &data, yPosition, layout.basicWidgetY);
}

/***** Seconds Widget *****/

int Seconds_getHeight() { return layout.secondsHeight; }

void Seconds_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  graphics_draw_text(ctx, currentSecondsNum, lgSidebarFont,
                     GRect(layout.textRectX + SidebarWidgets_xOffset,
                           yPosition + layout.secondsY, layout.textRectWidth,
                           text_rect_height(fontMetrics.lg)),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

/***** Weather Forecast Widget *****/

int WeatherForecast_getHeight() { return layout.weatherForecastHeight; }

void WeatherForecast_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  if (Weather_forecastWeatherIcon) {
    gdraw_command_image_recolor(Weather_forecastWeatherIcon,
                                dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);

    gdraw_command_image_draw(ctx, Weather_forecastWeatherIcon,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }

  // draw weather data only if it has been set
  if (Weather_weatherInfo.todaysHighTemp != INT32_MIN) {

    int todaysHighTemp = Weather_weatherInfo.todaysHighTemp;
    int todaysLowTemp = Weather_weatherInfo.todaysLowTemp;

    if (!settings.useMetric) {
      todaysHighTemp = roundf(todaysHighTemp * 1.8f + 32);
      todaysLowTemp = roundf(todaysLowTemp * 1.8f + 32);
    }

    char tempString[8];

    graphics_context_set_fill_color(ctx, settings.sidebarTextColor);

    // in large font mode, omit the degree symbol and move the text
    if (!settings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), "%d°", todaysHighTemp);
    } else {
      snprintf(tempString, sizeof(tempString), "%d", todaysHighTemp);
    }
    graphics_draw_text(ctx, tempString, currentSidebarFont,
                       GRect(layout.textRectX + SidebarWidgets_xOffset,
                             yPosition + layout.forecastHighY,
                             layout.textRectWidth, layout.textRectHeight),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    graphics_fill_rect(ctx,
                       GRect(layout.forecastDividerX + SidebarWidgets_xOffset,
                             8 + yPosition + layout.forecastDividerY,
                             layout.forecastDividerWidth, 1),
                       0, GCornerNone);

    if (!settings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), "%d°", todaysLowTemp);
    } else {
      snprintf(tempString, sizeof(tempString), "%d", todaysLowTemp);
    }
    graphics_draw_text(ctx, tempString, currentSidebarFont,
                       GRect(layout.textRectX + SidebarWidgets_xOffset,
                             yPosition + layout.forecastLowY,
                             layout.textRectWidth, layout.textRectHeight),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(ctx, "...", currentSidebarFont,
                       GRect(layout.textRectX + SidebarWidgets_xOffset,
                             yPosition, layout.textRectWidth,
                             layout.textRectHeight),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

/***** Alternate Time Zone Widget *****/

int AltTime_getHeight() { return layout.basicWidgetHeight; }

static BasicWidgetData AltTime_getBasicData() {
  return (BasicWidgetData){
      .label = settings.altclockName,
      .value = get_standard_text_data(altClock),
  };
}

void AltTime_draw(GContext *ctx, int yPosition) {
  BasicWidgetData data = AltTime_getBasicData();
  draw_basic_standard_widget(ctx, &data, yPosition, layout.basicWidgetY - 1);
}

/********** UV Index Widget **********/

int UVIndex_getHeight() { return layout.basicWidgetHeight; }

static BasicWidgetData UVIndex_getBasicData() {
  BasicWidgetData data = (BasicWidgetData){
      .label = "UV",
      .value = (StandardWidgetData){
          .available = true,
      },
  };

  if (Weather_weatherInfo.currentUVIndex != INT32_MIN) {
    snprintf(data.value.full, sizeof(data.value.full), "%d",
             (int)Weather_weatherInfo.currentUVIndex);
    snprintf(data.value.compact, sizeof(data.value.compact), "%s",
             data.value.full);
    snprintf(data.value.tiny, sizeof(data.value.tiny), "%s", data.value.full);
  } else {
    snprintf(data.value.full, sizeof(data.value.full), "...");
    snprintf(data.value.compact, sizeof(data.value.compact), "...");
    snprintf(data.value.tiny, sizeof(data.value.tiny), "...");
  }

  return data;
}

void UVIndex_draw(GContext *ctx, int yPosition) {
  BasicWidgetData data = UVIndex_getBasicData();
  draw_basic_standard_widget(ctx, &data, yPosition, layout.basicWidgetY);
}

#ifdef PBL_HEALTH

/***** Step Counter Widget *****/

int StepCounter_getHeight() { return layout.stepCounterHeight; }

static void format_step_candidates(int steps, StandardWidgetData *data) {
  snprintf(data->full, sizeof(data->full), "%i", steps);

  if (steps < 1000) {
    snprintf(data->compact, sizeof(data->compact), "%i", steps);
    snprintf(data->tiny, sizeof(data->tiny), "%i", steps);
    return;
  }

  int steps_thousands = steps / 1000;
  int steps_hundreds = steps / 100 % 10;

  if (steps < 10000) {
    snprintf(data->compact, sizeof(data->compact), "%i%c%ik", steps_thousands,
             settings.decimalSeparator, steps_hundreds);
  } else {
    snprintf(data->compact, sizeof(data->compact), "%ik", steps_thousands);
  }

  int rounded_thousands = (steps + 500) / 1000;
  snprintf(data->tiny, sizeof(data->tiny), "%ik", rounded_thousands);
}

static void format_distance_candidates(int distance, StandardWidgetData *data) {
  MeasurementSystem unit_system =
      health_service_get_measurement_system_for_display(
          HealthMetricWalkedDistanceMeters);

  if (unit_system == MeasurementSystemMetric) {
    if (distance < 1000) {
      snprintf(data->full, sizeof(data->full), "%im", distance);
      snprintf(data->compact, sizeof(data->compact), "%im", distance);
      snprintf(data->tiny, sizeof(data->tiny), "%im", distance);
    } else {
      int km_whole = distance / 1000;
      int km_tenths = distance / 100 % 10;
      snprintf(data->full, sizeof(data->full), "%i%c%ikm", km_whole,
               settings.decimalSeparator, km_tenths);
      snprintf(data->compact, sizeof(data->compact), "%ikm", km_whole);
      snprintf(data->tiny, sizeof(data->tiny), "%ik", km_whole);
    }
    return;
  }

  int miles_tenths_total = distance * 10 / 1609;
  int miles_whole = miles_tenths_total / 10;
  int miles_tenths = miles_tenths_total % 10;

  if (miles_whole > 0) {
    snprintf(data->full, sizeof(data->full), "%i%c%imi", miles_whole,
             settings.decimalSeparator, miles_tenths);
    snprintf(data->compact, sizeof(data->compact), "%imi", miles_whole);
    snprintf(data->tiny, sizeof(data->tiny), "%imi", miles_whole);
  } else {
    snprintf(data->full, sizeof(data->full), "%c%imi",
             settings.decimalSeparator, miles_tenths);
    snprintf(data->compact, sizeof(data->compact), "%c%imi",
             settings.decimalSeparator, miles_tenths);
    snprintf(data->tiny, sizeof(data->tiny), "%c%imi",
             settings.decimalSeparator, miles_tenths);
  }
}

static StandardWidgetData StepCounter_getStandardData() {
  StandardWidgetData data = (StandardWidgetData){
      .icon = stepsImage,
      .available = true,
  };

  if (settings.healthUseDistance) {
    int distance = 0;

    if (is_health_metric_accessible(HealthMetricWalkedDistanceMeters)) {
      distance =
          (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
    }

    format_distance_candidates(distance, &data);
  } else {
    int steps = 0;

    if (is_health_metric_accessible(HealthMetricStepCount)) {
      steps = (int)health_service_sum_today(HealthMetricStepCount);
    }

    format_step_candidates(steps, &data);
  }

  return data;
}

void StepCounter_draw(GContext *ctx, int yPosition) {
  StandardWidgetData data = StepCounter_getStandardData();
  int textWidth = SidebarWidgets_contentWidth;
  int textX = get_standard_widget_text_x(textWidth);

  StandardWidgetRenderContext renderCtx = (StandardWidgetRenderContext){
      .fitRect =
          get_standard_widget_fit_rect(textX, yPosition + layout.stepsTextY,
                                       textWidth),
      .drawRect = get_standard_widget_draw_rect(yPosition + layout.stepsTextY),
      .iconX = 3 + SidebarWidgets_xOffset,
      .iconY = yPosition - 7,
      .primaryFont = currentSidebarFont,
      .fallbackFont = fallbackSidebarFont,
  };

  draw_standard_widget(ctx, &data, renderCtx);
}

/***** Sleep Time Widget *****/

int SleepTimer_getHeight() { return layout.sleepTimerHeight; }

void SleepTimer_draw(GContext *ctx, int yPosition) {
  if (sleepImage) {
    gdraw_command_image_recolor(sleepImage, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, sleepImage,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition - 7));
  }

  // get sleep in seconds
  int sleep_seconds;

  HealthActivityMask metric = (settings.healthUseRestfulSleep)
                                  ? HealthMetricSleepRestfulSeconds
                                  : HealthMetricSleepSeconds;

  if (is_health_metric_accessible(metric)) {
    sleep_seconds = (int)health_service_sum_today(metric);
  } else {
    sleep_seconds = 0;
  }

  // convert to hours/minutes
  int sleep_minutes = sleep_seconds / 60;
  int sleep_hours = sleep_minutes / 60;

  // find minutes remainder
  sleep_minutes %= 60;

  char sleep_text[4];

  snprintf(sleep_text, sizeof(sleep_text), "%ih", sleep_hours);

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  graphics_draw_text(ctx, sleep_text, mdSidebarFont,
                     GRect(layout.textRectX + SidebarWidgets_xOffset,
                           yPosition + layout.sleepHoursY, layout.textRectWidth,
                           text_rect_height(fontMetrics.md)),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  snprintf(sleep_text, sizeof(sleep_text), "%im", sleep_minutes);

  graphics_draw_text(ctx, sleep_text, smSidebarFont,
                     GRect(layout.textRectX + SidebarWidgets_xOffset,
                           yPosition + layout.sleepMinutesY,
                           layout.textRectWidth, text_rect_height(fontMetrics.sm)),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

int HeartRate_getHeight() { return layout.heartRateHeight; }

static StandardWidgetData HeartRate_getStandardData() {
  StandardWidgetData data = (StandardWidgetData){
      .icon = heartImage,
      .available = is_health_metric_accessible(HealthMetricHeartRateBPM),
  };

  if (!data.available) {
    return data;
  }

  int heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);

  if (heart_rate <= 0) {
    data.available = false;
    return data;
  }

  snprintf(data.full, sizeof(data.full), "%i", heart_rate);
  snprintf(data.compact, sizeof(data.compact), "%i", heart_rate);
  snprintf(data.tiny, sizeof(data.tiny), "%i", heart_rate);

  return data;
}

void HeartRate_draw(GContext *ctx, int yPosition) {
  StandardWidgetData data = HeartRate_getStandardData();
  int textWidth = get_standard_widget_text_width();
  int textX = get_standard_widget_text_x(textWidth);

  StandardWidgetRenderContext renderCtx = (StandardWidgetRenderContext){
      .fitRect =
          get_standard_widget_fit_rect(textX, yPosition + layout.heartRateValueY,
                                       textWidth),
      .drawRect =
          get_standard_widget_draw_rect(yPosition + layout.heartRateValueY),
      .iconX = 3 + SidebarWidgets_xOffset,
      .iconY = yPosition,
      .primaryFont = currentSidebarFont,
      .fallbackFont = fallbackSidebarFont,
  };

  draw_standard_widget(ctx, &data, renderCtx);
}

#endif

/***** Beats (Swatch Internet Time) widget *****/

int Beats_getHeight() { return layout.basicWidgetHeight; }

static BasicWidgetData Beats_getBasicData() {
  return (BasicWidgetData){
      .label = "@",
      .value = get_standard_text_data(currentBeats),
  };
}

void Beats_draw(GContext *ctx, int yPosition) {
  BasicWidgetData data = Beats_getBasicData();
  draw_basic_standard_widget(ctx, &data, yPosition, layout.basicWidgetY - 1);
}
