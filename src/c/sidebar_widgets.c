#include "sidebar_widgets.h"
#include "languages.h"
#include "settings.h"
#include "util.h"
#include "weather.h"
#include <math.h>
#include <pebble.h>

bool SidebarWidgets_useCompactMode = false;
int SidebarWidgets_xOffset;

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
GFont batteryFont;

typedef struct {
  // battery meter
  int batteryGraphicOnlyHeight;
  int batteryWithPctHeight;
  int batteryTextY;
  // date
  int dateHeight, dateHeightCompact;
  int dateTopCorrection, dateDayNumY, dateMonthY;
  int dateBgX, dateBgY, dateBgRectY, dateBgRectInnerY;
  // current weather
  int weatherHeight;
  int weatherTempY;
  // common text rect width for main data display
  int textRectWidth;
  // weather forecast
  int weatherForecastHeight;
  int forecastHighY, forecastDividerY, forecastLowY;
  int forecastDividerX, forecastDividerWidth;
  // basic widgets (week number, alt time, UV index, beats)
  int basicWidgetHeight;
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
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_28);
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
  if (settings.useLargeFonts) {
    currentSidebarFont = lgSidebarFont;
    batteryFont = lgSidebarFont;
  } else {
    currentSidebarFont = mdSidebarFont;
    batteryFont = smSidebarFont;
  }

  if (settings.useLargeFonts) {
    layout = (SidebarWidgetLayout){
        .batteryGraphicOnlyHeight = 14,
        .batteryWithPctHeight = 33,
        .batteryTextY = 14,
        .dateHeight = 62,
        .dateHeightCompact = 42,
        .dateTopCorrection = 10,
        .dateDayNumY = 24,
        .dateMonthY = 48,
        .dateBgX = 3,
        .dateBgY = 23,
        .dateBgRectY = 30,
        .dateBgRectInnerY = 32,
        .weatherHeight = 44,
        .weatherTempY = 20,
        .textRectWidth = 40,
        .weatherForecastHeight = 63,
        .forecastHighY = 20,
        .forecastDividerY = 38,
        .forecastLowY = 39,
        .forecastDividerX = 3,
        .forecastDividerWidth = 24,
        .basicWidgetHeight = 29,
        .basicWidgetY = 6,
        .btDisconnectHeight = 22,
        .heartRateHeight = 40,
        .heartRateValueY = 17,
        .stepCounterHeight = 32,
        .stepsTextY = 13,
        .sleepTimerHeight = 44,
        .sleepHoursY = 14,
        .sleepMinutesY = 30,
        .secondsHeight = 14,
        .secondsY = -10,
    };
  } else {
    layout = (SidebarWidgetLayout){
        .batteryGraphicOnlyHeight = 14,
        .batteryWithPctHeight = 27,
        .batteryTextY = 18,
        .dateHeight = 58,
        .dateHeightCompact = 41,
        .dateTopCorrection = 7,
        .dateDayNumY = 26,
        .dateMonthY = 47,
        .dateBgX = 3,
        .dateBgY = 23,
        .dateBgRectY = 30,
        .dateBgRectInnerY = 32,
        .weatherHeight = 42,
        .weatherTempY = 24,
        .textRectWidth = 40,
        .weatherForecastHeight = 60,
        .forecastHighY = 24,
        .forecastDividerY = 37,
        .forecastLowY = 42,
        .forecastDividerX = 3,
        .forecastDividerWidth = 24,
        .basicWidgetHeight = 26,
        .basicWidgetY = 9,
        .btDisconnectHeight = 22,
        .heartRateHeight = 38,
        .heartRateValueY = 21,
        .stepCounterHeight = 36,
        .stepsTextY = 13,
        .sleepTimerHeight = 44,
        .sleepHoursY = 14,
        .sleepMinutesY = 30,
        .secondsHeight = 14,
        .secondsY = -10,
    };
  }

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  // Larger fonts on these platforms — override layout values.
  // These start from the standard large-font values as a baseline; tune as
  // needed.
  if (settings.useLargeFonts) {
    layout.batteryWithPctHeight = 36;
    layout.batteryTextY = 11;
    layout.dateHeight = 66;
    layout.dateHeightCompact = 45;
    layout.dateTopCorrection = 12;
    layout.dateDayNumY = 22;
    layout.dateMonthY = 51;
    layout.dateBgX = 2;
    layout.dateBgY = 23;
    layout.dateBgRectY = 32;
    layout.dateBgRectInnerY = 34;
    layout.weatherHeight = 47;
    layout.weatherTempY = 18;
    layout.textRectWidth = 40;
    layout.weatherForecastHeight = 67;
    layout.forecastHighY = 18;
    layout.forecastDividerY = 40;
    layout.forecastLowY = 37;
    layout.forecastDividerX = 1;
    layout.forecastDividerWidth = 28;
    layout.basicWidgetHeight = 32;
    layout.basicWidgetY = 4;
    layout.btDisconnectHeight = 22;
    layout.heartRateHeight = 43;
    layout.heartRateValueY = 14;
    layout.stepCounterHeight = 32;
    layout.stepsTextY = 11;
    layout.sleepTimerHeight = 44;
    layout.sleepHoursY = 12;
    layout.sleepMinutesY = 29;
    layout.secondsHeight = 18;
    layout.secondsY = -12;
  } else {
    layout.batteryWithPctHeight = 30;
    layout.batteryTextY = 17;
    layout.dateHeight = 67;
    layout.dateHeightCompact = 42;
    layout.dateTopCorrection = 16;
    layout.dateDayNumY = 29;
    layout.dateMonthY = 53;
    layout.dateBgX = 1;
    layout.dateBgY = 29;
    layout.dateBgRectY = 35;
    layout.dateBgRectInnerY = 35;
    layout.weatherHeight = 46;
    layout.weatherTempY = 22;
    layout.textRectWidth = 40;
    layout.weatherForecastHeight = 69;
    layout.forecastHighY = 22;
    layout.forecastDividerY = 42;
    layout.forecastLowY = 45;
    layout.forecastDividerX = 1;
    layout.forecastDividerWidth = 28;
    layout.basicWidgetHeight = 32;
    layout.basicWidgetY = 11;
    layout.btDisconnectHeight = 22;
    layout.heartRateHeight = 40;
    layout.heartRateValueY = 20;
    layout.stepCounterHeight = 32;
    layout.stepsTextY = 11;
    layout.sleepTimerHeight = 52;
    layout.sleepHoursY = 13;
    layout.sleepMinutesY = 34;
    layout.secondsHeight = 18;
    layout.secondsY = -10;
  }
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

void BatteryMeter_draw(GContext *ctx, int yPosition) {

  BatteryChargeState chargeState = battery_state_service_peek();
  uint8_t battery_percent =
      (chargeState.charge_percent > 0) ? chargeState.charge_percent : 5;

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  char batteryString[6];
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
    if (!settings.useLargeFonts) {
      // put the percent sign on the opposite side if turkish
      snprintf(batteryString, sizeof(batteryString),
               (settings.languageId == LANGUAGE_TR) ? "%%%d" : "%d%%",
               battery_percent);
    } else {
      snprintf(batteryString, sizeof(batteryString), "%d", battery_percent);
    }
    graphics_draw_text(ctx, batteryString, batteryFont,
                       GRect(-5 + SidebarWidgets_xOffset,
                             layout.batteryTextY + batteryPositionY,
                             layout.textRectWidth, 20),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
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
  graphics_draw_text(
      ctx, currentDayName, currentSidebarFont,
      GRect(-5 + SidebarWidgets_xOffset, yPosition, layout.textRectWidth, 20),
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
                       GRect(2 + SidebarWidgets_xOffset,
                             yPosition + layout.dateBgRectY, 26, 22),
                       2, GCornersAll);

    graphics_context_set_fill_color(ctx, dynamicSettings.iconFillColor);
    graphics_fill_rect(ctx,
                       GRect(4 + SidebarWidgets_xOffset,
                             yPosition + layout.dateBgRectInnerY, 22, 18),
                       0, GCornersAll);
  }

  // next, draw the date number
  graphics_context_set_text_color(ctx, dynamicSettings.iconStrokeColor);

  int yOffset = layout.dateDayNumY;

  graphics_draw_text(ctx, currentDayNum, currentSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset, yPosition + yOffset,
                           layout.textRectWidth, 20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  // switch back to normal color for the rest
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  // don't draw the month if we're in compact mode
  if (!SidebarWidgets_useCompactMode) {
    yOffset = layout.dateMonthY;

    graphics_draw_text(ctx, currentMonth, currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset, yPosition + yOffset,
                             layout.textRectWidth, 20),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

/********** current weather widget **********/

int CurrentWeather_getHeight() { return layout.weatherHeight; }

void CurrentWeather_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  if (Weather_currentWeatherIcon) {
    gdraw_command_image_recolor(Weather_currentWeatherIcon,
                                dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, Weather_currentWeatherIcon,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }

  // draw weather data only if it has been set
  if (Weather_weatherInfo.currentTemp != INT32_MIN) {

    int currentTemp = Weather_weatherInfo.currentTemp;

    if (!settings.useMetric) {
      currentTemp = roundf(currentTemp * 1.8f + 32);
    }

    char tempString[8];

    // in large font mode, omit the degree symbol and move the text
    if (!settings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", currentTemp);
    } else {
      snprintf(tempString, sizeof(tempString), " %d", currentTemp);
    }
    graphics_draw_text(ctx, tempString, currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset,
                             yPosition + layout.weatherTempY,
                             layout.textRectWidth, 20),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(
        ctx, "...", currentSidebarFont,
        GRect(-5 + SidebarWidgets_xOffset, yPosition, layout.textRectWidth, 20),
        GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
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

static void draw_basic_widget(GContext *ctx, int yPosition, const char *label,
                              const char *value, int labelYOffset,
                              int valueYOffset) {
  graphics_draw_text(ctx, label, smSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset,
                           yPosition + labelYOffset, layout.textRectWidth, 20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, value, currentSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset,
                           yPosition + valueYOffset, layout.textRectWidth, 20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

/***** Week Number Widget *****/

int WeekNumber_getHeight() { return layout.basicWidgetHeight; }

void WeekNumber_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  draw_basic_widget(ctx, yPosition, wordForWeek[settings.languageId],
                    currentWeekNum, -4, layout.basicWidgetY);
}

/***** Seconds Widget *****/

int Seconds_getHeight() { return layout.secondsHeight; }

void Seconds_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  graphics_draw_text(ctx, currentSecondsNum, lgSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset,
                           yPosition + layout.secondsY, layout.textRectWidth,
                           20),
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
      snprintf(tempString, sizeof(tempString), " %d°", todaysHighTemp);
    } else {
      snprintf(tempString, sizeof(tempString), "%d", todaysHighTemp);
    }
    graphics_draw_text(ctx, tempString, currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset,
                             yPosition + layout.forecastHighY,
                             layout.textRectWidth, 20),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    graphics_fill_rect(ctx,
                       GRect(layout.forecastDividerX + SidebarWidgets_xOffset,
                             8 + yPosition + layout.forecastDividerY,
                             layout.forecastDividerWidth, 1),
                       0, GCornerNone);

    if (!settings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", todaysLowTemp);
    } else {
      snprintf(tempString, sizeof(tempString), "%d", todaysLowTemp);
    }
    graphics_draw_text(ctx, tempString, currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset,
                             yPosition + layout.forecastLowY,
                             layout.textRectWidth, 20),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(
        ctx, "...", currentSidebarFont,
        GRect(-5 + SidebarWidgets_xOffset, yPosition, layout.textRectWidth, 20),
        GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

/***** Alternate Time Zone Widget *****/

int AltTime_getHeight() { return layout.basicWidgetHeight; }

void AltTime_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  draw_basic_widget(ctx, yPosition, settings.altclockName, altClock, -5,
                    layout.basicWidgetY - 1);
}

/********** UV Index Widget **********/

int UVIndex_getHeight() { return layout.basicWidgetHeight; }

void UVIndex_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  char uvString[5];
  if (Weather_weatherInfo.currentUVIndex != INT32_MIN) {
    snprintf(uvString, sizeof(uvString), "%d",
             (int)Weather_weatherInfo.currentUVIndex);
  } else {
    snprintf(uvString, sizeof(uvString), "...");
  }
  draw_basic_widget(ctx, yPosition, "UV", uvString, -4, layout.basicWidgetY);
}

#ifdef PBL_HEALTH

/***** Step Counter Widget *****/

int StepCounter_getHeight() { return layout.stepCounterHeight; }

void StepCounter_draw(GContext *ctx, int yPosition) {
  if (stepsImage) {
    gdraw_command_image_recolor(stepsImage, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, stepsImage,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition - 7));
  }

  char steps_text[8];
  bool use_small_font = false;

  if (settings.healthUseDistance) {
    int distance = 0;

    if (is_health_metric_accessible(HealthMetricWalkedDistanceMeters)) {
      distance =
          (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
    }

    MeasurementSystem unit_system =
        health_service_get_measurement_system_for_display(
            HealthMetricWalkedDistanceMeters);

    // format distance string
    if (unit_system == MeasurementSystemMetric) {
      if (distance < 100) {
        snprintf(steps_text, sizeof(steps_text), "%im", distance);
      } else if (distance < 1000) {
        distance /= 100; // convert to tenths of km
        snprintf(steps_text, sizeof(steps_text), ".%ikm", distance);
      } else {
        distance /= 1000; // convert to km

        if (distance > 9) {
          use_small_font = true;
        }

        snprintf(steps_text, sizeof(steps_text), "%ikm", distance);
      }
    } else {
      int miles_tenths = distance * 10 / 1609 % 10;
      int miles_whole = (int)roundf(distance / 1609.0f);

      if (miles_whole > 0) {
        snprintf(steps_text, sizeof(steps_text), "%imi", miles_whole);
      } else {
        snprintf(steps_text, sizeof(steps_text), "%c%imi",
                 settings.decimalSeparator, miles_tenths);
      }
    }
  } else {
    int steps = (int)health_service_sum_today(HealthMetricStepCount);

    if (is_health_metric_accessible(HealthMetricStepCount)) {
      steps = (int)health_service_sum_today(HealthMetricStepCount);
    }

    // format step string
    if (steps < 1000) {
      snprintf(steps_text, sizeof(steps_text), "%i", steps);
    } else {
      int steps_thousands = steps / 1000;
      int steps_hundreds = steps / 100 % 10;

      if (steps < 10000) {
        snprintf(steps_text, sizeof(steps_text), "%i%c%ik", steps_thousands,
                 settings.decimalSeparator, steps_hundreds);
      } else {
        snprintf(steps_text, sizeof(steps_text), "%ik", steps_thousands);
      }
    }
  }

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  graphics_draw_text(
      ctx, steps_text, (use_small_font) ? smSidebarFont : mdSidebarFont,
      GRect(-5 + SidebarWidgets_xOffset, yPosition + layout.stepsTextY,
            layout.textRectWidth, 20),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
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
                     GRect(-5 + SidebarWidgets_xOffset,
                           yPosition + layout.sleepHoursY, layout.textRectWidth,
                           20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  snprintf(sleep_text, sizeof(sleep_text), "%im", sleep_minutes);

  graphics_draw_text(ctx, sleep_text, smSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset,
                           yPosition + layout.sleepMinutesY,
                           layout.textRectWidth, 20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

int HeartRate_getHeight() { return layout.heartRateHeight; }

void HeartRate_draw(GContext *ctx, int yPosition) {
  if (heartImage) {
    gdraw_command_image_recolor(heartImage, dynamicSettings.iconFillColor,
                                dynamicSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, heartImage,
                             GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }

  int yOffset = layout.heartRateValueY;

  // TODO: accessibility check?
  int heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
  char heart_rate_text[8];

  snprintf(heart_rate_text, sizeof(heart_rate_text), "%i", heart_rate);

  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  graphics_draw_text(ctx, heart_rate_text, currentSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset, yPosition + yOffset,
                           layout.textRectWidth, 20),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

#endif

/***** Beats (Swatch Internet Time) widget *****/

int Beats_getHeight() { return layout.basicWidgetHeight; }

void Beats_draw(GContext *ctx, int yPosition) {
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);
  draw_basic_widget(ctx, yPosition, "@", currentBeats, -5,
                    layout.basicWidgetY - 1);
}
