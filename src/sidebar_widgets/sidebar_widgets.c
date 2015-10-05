#include <pebble.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar_widgets.h"

bool SidebarWidgets_useCompactMode = false;

// sidebar icons
#ifdef PBL_COLOR
  GDrawCommandImage* dateImage;
  GDrawCommandImage* disconnectImage;
  GDrawCommandImage* batteryImage;
  GDrawCommandImage* batteryChargeImage;
#else
  GBitmap* dateImage;
  GBitmap* disconnectImage;
  GBitmap* batteryImage;
  GBitmap* batteryChargeImage;
#endif

// fonts
GFont smSidebarFont;
GFont mdSidebarFont;
GFont lgSidebarFont;
GFont currentSidebarFont;
GFont batteryFont;

// the date and weather strings
char currentDayName[8];
char currentDayNum[8];
char currentMonth[8];
char currentWeekNum[8];
char currentSecondsNum[8];
char altClock[8];

// the widgets
SidebarWidget batteryMeterWidget;
int BatteryMeter_getHeight();
void BatteryMeter_draw(GContext* ctx, int yPosition);

SidebarWidget emptyWidget;
int EmptyWidget_getHeight();
void EmptyWidget_draw(GContext* ctx, int yPosition);

SidebarWidget dateWidget;
int DateWidget_getHeight();
void DateWidget_draw(GContext* ctx, int yPosition);

SidebarWidget currentWeatherWidget;
int CurrentWeather_getHeight();
void CurrentWeather_draw(GContext* ctx, int yPosition);

SidebarWidget weatherForecastWidget;
int WeatherForecast_getHeight();
void WeatherForecast_draw(GContext* ctx, int yPosition);

SidebarWidget btDisconnectWidget;
int BTDisconnect_getHeight();
void BTDisconnect_draw(GContext* ctx, int yPosition);

SidebarWidget weekNumberWidget;
int WeekNumber_getHeight();
void WeekNumber_draw(GContext* ctx, int yPosition);

SidebarWidget secondsWidget;
int Seconds_getHeight();
void Seconds_draw(GContext* ctx, int yPosition);

SidebarWidget altTimeWidget;
int AltTime_getHeight();
void AltTime_draw(GContext* ctx, int yPosition);

void SidebarWidgets_init() {
  // load fonts
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // load the sidebar graphics
  #ifdef PBL_COLOR
    dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
    disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
    batteryImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
    batteryChargeImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  #else
    dateImage = gbitmap_create_with_resource(RESOURCE_ID_DATE_BG);
    disconnectImage = gbitmap_create_with_resource(RESOURCE_ID_DISCONNECTED);
    batteryImage = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_BG);
    batteryChargeImage = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  #endif

  // set up widgets' function pointers correctly
  batteryMeterWidget.getHeight = BatteryMeter_getHeight;
  batteryMeterWidget.draw      = BatteryMeter_draw;

  emptyWidget.getHeight = EmptyWidget_getHeight;
  emptyWidget.draw      = EmptyWidget_draw;

  dateWidget.getHeight = DateWidget_getHeight;
  dateWidget.draw      = DateWidget_draw;

  currentWeatherWidget.getHeight = CurrentWeather_getHeight;
  currentWeatherWidget.draw      = CurrentWeather_draw;

  weatherForecastWidget.getHeight = WeatherForecast_getHeight;
  weatherForecastWidget.draw      = WeatherForecast_draw;

  btDisconnectWidget.getHeight = BTDisconnect_getHeight;
  btDisconnectWidget.draw      = BTDisconnect_draw;

  weekNumberWidget.getHeight = WeekNumber_getHeight;
  weekNumberWidget.draw      = WeekNumber_draw;

  secondsWidget.getHeight = Seconds_getHeight;
  secondsWidget.draw      = Seconds_draw;

  altTimeWidget.getHeight = AltTime_getHeight;
  altTimeWidget.draw      = AltTime_draw;

}

void SidebarWidgets_deinit() {
  #ifdef PBL_COLOR
    gdraw_command_image_destroy(dateImage);
    gdraw_command_image_destroy(disconnectImage);
    gdraw_command_image_destroy(batteryImage);
    gdraw_command_image_destroy(batteryChargeImage);
  #else
    gbitmap_destroy(dateImage);
    gbitmap_destroy(disconnectImage);
    gbitmap_destroy(batteryImage);
    gbitmap_destroy(batteryChargeImage);
  #endif
}

void SidebarWidgets_updateFonts() {
  if(globalSettings.useLargeFonts) {
    currentSidebarFont = lgSidebarFont;
    batteryFont = lgSidebarFont;
  } else {
    currentSidebarFont = mdSidebarFont;
    batteryFont = smSidebarFont;
  }
}

// c can't do true modulus on negative numbers, apparently
// from http://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void SidebarWidgets_updateTime(struct tm* timeInfo) {
  // set all the date strings
  strftime(currentDayNum,  3, "%e", timeInfo);
  strftime(currentWeekNum, 3, "%U", timeInfo);

  // set the seconds string
  strftime(currentSecondsNum, 4, ":%S", timeInfo);

  // set the alternate time zone string
  int hour = timeInfo->tm_hour;

  // apply the configured offset value
  hour += globalSettings.altclockOffset;

  // format it
  if(clock_is_24h_style()) {
    hour = mod(hour, 24);
  } else {
    hour = mod(hour, 12);

    if(hour == 0) {
      hour = 12;
    }
  }

  if(globalSettings.showLeadingZero && hour < 10) {
    snprintf(altClock, sizeof(altClock), "0%i", hour);
  } else {
    snprintf(altClock, sizeof(altClock), "%i", hour);
  }

  strncpy(currentDayName, dayNames[globalSettings.languageId][timeInfo->tm_wday], sizeof(currentDayName));
  strncpy(currentMonth, monthNames[globalSettings.languageId][timeInfo->tm_mon], sizeof(currentMonth));

  // remove padding on date num, if needed
  if(currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }
}

/* Sidebar Widget Selection */
SidebarWidget getSidebarWidgetByType(SidebarWidgetType type) {
  switch(type) {
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
    default:
      return emptyWidget;
      break;
  }
}

/********** functions for the empty widget **********/
int EmptyWidget_getHeight() {
  return 0;
}

void EmptyWidget_draw(GContext* ctx, int yPosition) {
  return;
}

/********** functions for the battery meter widget **********/

int BatteryMeter_getHeight() {
  if(globalSettings.showBatteryPct) {
    return (globalSettings.useLargeFonts) ? 33 : 27;
  } else {
    return 14; // graphic-only height
  }
}

void BatteryMeter_draw(GContext* ctx, int yPosition) {
  BatteryChargeState chargeState = battery_state_service_peek();

  char batteryString[6];

  int batteryPositionY = yPosition - 5; // correct for vertical empty space on battery icon

  if(chargeState.is_charging) {
    if(batteryChargeImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, batteryChargeImage, GPoint(3, batteryPositionY));
      #else
        graphics_draw_bitmap_in_rect(ctx, batteryChargeImage, GRect(3, batteryPositionY, 25, 25));
      #endif
    }
  } else {
    if (batteryImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, batteryImage, GPoint(3, batteryPositionY));
      #else
        graphics_draw_bitmap_in_rect(ctx, batteryImage, GRect(3, batteryPositionY, 25, 25));
      #endif
    }

    int width = roundf(18 * chargeState.charge_percent / 100.0f);

    graphics_context_set_fill_color(ctx, GColorBlack);

    #ifdef PBL_COLOR
      if(chargeState.charge_percent <= 20) {
        graphics_context_set_fill_color(ctx, GColorRed);
      }
    #else
      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorWhite);
      }
    #endif

    graphics_fill_rect(ctx, GRect(6, 8 + batteryPositionY, width, 8), 0, GCornerNone);
  }

  // never show battery % while charging, because of this issue:
  // https://github.com/freakified/TimeStylePebble/issues/11
  if(globalSettings.showBatteryPct && !chargeState.is_charging) {
    if(!globalSettings.useLargeFonts) {
      snprintf(batteryString, sizeof(batteryString), "%d%%", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4, 18 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(batteryString, sizeof(batteryString), "%d", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4, 14 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  }
}

/********** current date widget **********/

int DateWidget_getHeight() {
  if(globalSettings.useLargeFonts) {
    return 62;
  } else  {
    return 58;
  }
}

void DateWidget_draw(GContext* ctx, int yPosition) {
  // hack to compensate for extra space that appears on the top of the date widget
  yPosition -= (globalSettings.useLargeFonts) ? 10 : 7;

  // first draw the day name
  graphics_draw_text(ctx,
                     currentDayName,
                     currentSidebarFont,
                     GRect(-5, yPosition, 40, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  // next, draw the date background
  // (an image in normal mode, a rectangle in large font mode)
  if(!globalSettings.useLargeFonts) {
    if (dateImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, dateImage, GPoint(3, yPosition + 23));
      #else
        graphics_draw_bitmap_in_rect(ctx, dateImage, GRect(3, yPosition + 23, 25, 25));
      #endif
    }
  } else {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(2, yPosition + 30, 26, 22), 2, GCornersAll);
    #else
      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorWhite);
      } else {
        graphics_context_set_fill_color(ctx, GColorBlack);
      }

      graphics_fill_rect(ctx, GRect(1, yPosition + 23, 28, 22), 2, GCornersAll);

      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorBlack);
      } else {
        graphics_context_set_fill_color(ctx, GColorWhite);
      }

      graphics_fill_rect(ctx, GRect(3, yPosition + 6, 24, 18), 0, GCornersAll);

    #endif

  }

  // next, draw the date number

  // color pebble should always use black for the date number
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif

  int yOffset = 0;
  yOffset = globalSettings.useLargeFonts ? 24 : 26;

  graphics_draw_text(ctx,
                     currentDayNum,
                     currentSidebarFont,
                     GRect(0, yPosition + yOffset, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);


   // switch back to normal color for the rest
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);
  #endif

  yOffset = globalSettings.useLargeFonts ? 48 : 47;

  graphics_draw_text(ctx,
                     currentMonth,
                     currentSidebarFont,
                     GRect(0, yPosition + yOffset, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

}

/********** current weather widget **********/

int CurrentWeather_getHeight() {
  if(globalSettings.useLargeFonts) {
    return 44;
  } else {
    return 42;
  }
}

void CurrentWeather_draw(GContext* ctx, int yPosition) {

  if (Weather_currentWeatherIcon) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3, yPosition));
    #else
      graphics_draw_bitmap_in_rect(ctx, Weather_currentWeatherIcon, GRect(3, yPosition, 25, 25));
    #endif
  }

  // draw weather data only if it has been set
  if(Weather_weatherInfo.currentTemp != INT32_MIN) {

    int currentTemp = Weather_weatherInfo.currentTemp;

    if(!globalSettings.useMetric) {
      currentTemp = roundf(currentTemp * 1.8f + 32);
    }

    char tempString[8];

    // in large font mode, omit the degree symbol and move the text
    if(!globalSettings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", currentTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 24, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(tempString, sizeof(tempString), " %d", currentTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 20, 35, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(ctx,
                       "...",
                       currentSidebarFont,
                       GRect(-5, yPosition, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Bluetooth Disconnection Widget *****/

int BTDisconnect_getHeight() {
  return 22;
}

void BTDisconnect_draw(GContext* ctx, int yPosition) {
  if(disconnectImage) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, disconnectImage, GPoint(3, yPosition));
    #else
      graphics_draw_bitmap_in_rect(ctx, disconnectImage, GRect(3, yPosition, 25, 25));
    #endif
  }
}

/***** Week Number Widget *****/

int WeekNumber_getHeight() {
  return (globalSettings.useLargeFonts) ? 27 : 25;
}

void WeekNumber_draw(GContext* ctx, int yPosition) {
  // note that it draws "above" the y position to correct for
  // the vertical padding
  graphics_draw_text(ctx,
                     wordForWeek[globalSettings.languageId],
                     smSidebarFont,
                     GRect(0, yPosition - 4, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  if(!globalSettings.useLargeFonts) {
    graphics_draw_text(ctx,
                       currentWeekNum,
                       mdSidebarFont,
                       GRect(0, yPosition + 8, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  } else {
    graphics_draw_text(ctx,
                       currentWeekNum,
                       lgSidebarFont,
                       GRect(0, yPosition + 4, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Seconds Widget *****/

int Seconds_getHeight() {
  return 14;
}

void Seconds_draw(GContext* ctx, int yPosition) {
  graphics_draw_text(ctx,
                     currentSecondsNum,
                     lgSidebarFont,
                     GRect(0, yPosition - 10, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

/***** Weather Forecast Widget *****/

int WeatherForecast_getHeight() {
  if(globalSettings.useLargeFonts) {
    return 63;
  } else {
    return 60;
  }
}

void WeatherForecast_draw(GContext* ctx, int yPosition) {
  if(Weather_forecastWeatherIcon) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, Weather_forecastWeatherIcon, GPoint(3, yPosition));
    #else
      graphics_draw_bitmap_in_rect(ctx, Weather_forecastWeatherIcon, GRect(3, yPosition, 25, 25));
    #endif
  }

  // draw weather data only if it has been set
  if(Weather_weatherForecast.highTemp != INT32_MIN) {

    int highTemp = Weather_weatherForecast.highTemp;
    int lowTemp  = Weather_weatherForecast.lowTemp;

    if(!globalSettings.useMetric) {
      highTemp = roundf(highTemp * 1.8f + 32);
      lowTemp  = roundf(lowTemp * 1.8f + 32);
    }

    char tempString[8];

    graphics_context_set_fill_color(ctx, globalSettings.sidebarTextColor);

    // in large font mode, omit the degree symbol and move the text
    if(!globalSettings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", highTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 24, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);

      graphics_fill_rect(ctx, GRect(3, 8 + yPosition + 37, 24, 1), 0, GCornerNone);

      snprintf(tempString, sizeof(tempString), " %d°", lowTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 42, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(tempString, sizeof(tempString), " %d", highTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 20, 35, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);

      graphics_fill_rect(ctx, GRect(3, 8 + yPosition + 38, 24, 1), 0, GCornerNone);

      snprintf(tempString, sizeof(tempString), " %d", lowTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5, yPosition + 39, 35, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(ctx,
                       "...",
                       currentSidebarFont,
                       GRect(-5, yPosition, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Alternate Time Zone Widget *****/

int AltTime_getHeight() {
  return (globalSettings.useLargeFonts) ? 28 : 26;
}

void AltTime_draw(GContext* ctx, int yPosition) {


  graphics_draw_text(ctx,
                     globalSettings.altclockName,
                     smSidebarFont,
                     GRect(0, yPosition - 7, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  int yMod = (globalSettings.useLargeFonts) ? 2 : 6;

  graphics_draw_text(ctx,
                     altClock,
                     currentSidebarFont,
                     GRect(-1, yPosition + yMod, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}
