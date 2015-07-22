#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "clock_digit.h"
#include "weather.h"
#include "messaging.h"
#include "settings.h"
#include "languages.h"

#define FORCE_BACKLIGHT false

// windows and layers
static Window* mainWindow;
static Layer* sidebarLayer;

// PDC images
#ifdef PBL_COLOR
  static GDrawCommandImage* dateImage;
  static GDrawCommandImage* disconnectImage;
  static GDrawCommandImage* batteryImage;
  static GDrawCommandImage* batteryChargeImage;
#else
  static GBitmap* dateImage;
  static GBitmap* disconnectImage;
  static GBitmap* batteryImage;
  static GBitmap* batteryChargeImage;
#endif

// current bluetooth state
static bool isPhoneConnected;

// fonts
static GFont sidebarFont;
static GFont batteryFont;
static GFont dateFont;

// the four digits on the clock, ordered h1 h2, m1 m2
static ClockDigit clockDigits[4];

// the date and weather strings
static char currentDayName[8];
static char currentDayNum[8];
static char currentMonth[8];

void update_clock() {
  time_t rawTime;
  struct tm* timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  // DEBUG: use fake time for screenshots
//   if(SCREENSHOT_MODE) {
//     timeInfo->tm_hour = 3;
//     timeInfo->tm_min = 25;
//   }

  int hour = timeInfo->tm_hour;

  if (!clock_is_24h_style()) {
    if(hour > 12) {
      hour %= 12;
    } else if(timeInfo->tm_hour == 0) {
      hour = 12;
    }
  }

  // use the blank image for the leading hour digit if needed
  if(Settings_showLeadingZero || hour / 10 != 0) {
    ClockDigit_setNumber(&clockDigits[0], hour / 10);
  } else {
    ClockDigit_setBlank(&clockDigits[0]);
  }

  ClockDigit_setNumber(&clockDigits[1], hour % 10);
  ClockDigit_setNumber(&clockDigits[2], timeInfo->tm_min  / 10);
  ClockDigit_setNumber(&clockDigits[3], timeInfo->tm_min  % 10);

  // set all the date strings
  strftime(currentDayNum,  3, "%e", timeInfo);

  strncpy(currentDayName, dayNames[globalSettings.languageId][timeInfo->tm_wday], sizeof(currentDayName));
  strncpy(currentMonth, monthNames[globalSettings.languageId][timeInfo->tm_mon], sizeof(currentMonth));
  printf("language id: %i current month: %s current day: %s", globalSettings.languageId, currentMonth, currentDayName);

  // remove padding on date num, if needed
  if(currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }

  layer_mark_dirty(sidebarLayer);
}

void drawBatteryStatus(GContext* ctx) {
  BatteryChargeState chargeState = battery_state_service_peek();
  char batteryString[6];

  int batteryPositionY = 63;

  if(Settings_showBatteryPct) {
    batteryPositionY = 58;
  }

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

  if(Settings_showBatteryPct) {
    snprintf(batteryString, sizeof(batteryString), "%d%%", chargeState.charge_percent);

    graphics_draw_text(ctx,
                       batteryString,
                       batteryFont,
                       GRect(-4, 18 + batteryPositionY, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

void sidebarLayerUpdateProc(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // on black and white pebbles, invert the icons if we're using the dark bar
  #ifndef PBL_COLOR
    if(globalSettings.sidebarTextColor == GColorWhite) {
      graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
    } else {
      graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    }
  #endif

  if (Weather_currentWeatherIcon) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3, 7));
    #else
      graphics_draw_bitmap_in_rect(ctx, Weather_currentWeatherIcon, GRect(3, 7, 25, 25));
    #endif
  }

  // draw weather data only if it has been set
  if(Weather_weatherInfo.currentTemp != INT32_MIN) {

    int currentTemp = Weather_weatherInfo.currentTemp;

    if(!globalSettings.useMetric) {
      currentTemp = roundf((currentTemp * 9.0f) / 5.0f + 32);
    }

    char tempString[8];
    snprintf(tempString, sizeof(tempString), " %d°", currentTemp);

    graphics_draw_text(ctx,
                       tempString,
                       sidebarFont,
                       GRect(-5, 31, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }

  // if the pebble is disconnected, display the disconnection image
  if (disconnectImage && !isPhoneConnected) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, disconnectImage, GPoint(3, 60));
    #else
      graphics_draw_bitmap_in_rect(ctx, disconnectImage, GRect(3, 60, 25, 25));
    #endif
  } else {
    // otherwise, display the battery life, if enabled
    if(globalSettings.showBatteryLevel) {
      drawBatteryStatus(ctx);
    }
  }

  // now draw in the date info
  graphics_draw_text(ctx,
                     currentDayName,
                     sidebarFont,
                     GRect(0, 95, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  if (dateImage) {
    #ifdef PBL_COLOR
      gdraw_command_image_draw(ctx, dateImage, GPoint(3, 118));
    #else
      graphics_draw_bitmap_in_rect(ctx, dateImage, GRect(3, 118, 25, 25));
    #endif
  }

  // color pebble should always use black for the date number...
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif

  graphics_draw_text(ctx,
                     currentDayNum,
                     dateFont,
                     GRect(4, 121, 22, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  // switch back to normal color for the month
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);
  #endif

  graphics_draw_text(ctx,
                     currentMonth,
                     sidebarFont,
                     GRect(0, 142, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

void redrawSidebar() {
  layer_mark_dirty(sidebarLayer);
}

/* forces everything on screen to be redrawn -- perfect for keeping track of settings! */
void forceScreenRedraw() {
  // maybe the colors changed!
  for(int i = 0; i < 4; i++) {
    ClockDigit_setColor(&clockDigits[i], globalSettings.timeColor, globalSettings.timeBgColor);
  }

  window_set_background_color(mainWindow, globalSettings.timeBgColor);

  // or maybe the sidebar position changed!
  if(globalSettings.sidebarOnRight) {
    layer_set_frame(sidebarLayer, GRect(114, 0, 30, 168));

    for(int i = 0; i < 4; i++) {
      ClockDigit_offsetPosition(&clockDigits[i], 0);
    }
  } else {
    layer_set_frame(sidebarLayer, GRect(0, 0, 30, 168));

    for(int i = 0; i < 4; i++) {
      ClockDigit_offsetPosition(&clockDigits[i], 30);
    }
  }

  // maybe the language changed!
  update_clock();
}
static void main_window_load(Window *window) {
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "trying to construct");
  ClockDigit_construct(&clockDigits[0], GPoint(7, 7));
  ClockDigit_construct(&clockDigits[1], GPoint(60, 7));
  ClockDigit_construct(&clockDigits[2], GPoint(7, 90));
  ClockDigit_construct(&clockDigits[3], GPoint(60, 90));

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Made it past construction");

  for(int i = 0; i < 4; i++) {
    ClockDigit_setColor(&clockDigits[i], globalSettings.timeColor, globalSettings.timeBgColor);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clockDigits[i].imageLayer));
  }

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Made it past color setting");

  // load font
  sidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  batteryFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  dateFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // init the sidebar layer
  if(globalSettings.sidebarOnRight) {
    sidebarLayer = layer_create(GRect(114, 0, 30, 168));
  } else {
    sidebarLayer = layer_create(GRect(0, 0, 30, 168));
  }
  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, sidebarLayerUpdateProc);

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

  if (!dateImage || !disconnectImage) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load a PDC image.");
  }

  window_set_background_color(window, globalSettings.timeBgColor);

  // Make sure the time is displayed from the start
  forceScreenRedraw();
  update_clock();
}

static void main_window_unload(Window *window) {
  for(int i = 0; i < 4; i++) {
    ClockDigit_destruct(&clockDigits[i]);
  }

  layer_destroy(sidebarLayer);

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

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // every 30 minutes, request new weather data
  if(tick_time->tm_min % 30 == 0) {
    messaging_requestNewWeatherData();
  }

  update_clock();
}

void bluetoothStateChanged(bool newConnectionState){
  // if the phone was connected but isn't anymore and the user has opted in,
  // trigger a vibration
  if(isPhoneConnected && !newConnectionState && globalSettings.btVibe) {
    vibes_short_pulse();
  }

  isPhoneConnected = newConnectionState;

  redrawSidebar();
}

void batteryStateChanged(BatteryChargeState charge_state) {
  redrawSidebar();
}


static void init() {
  setlocale(LC_ALL, "");

  if(FORCE_BACKLIGHT) {
    light_enable(true);
  }

  // init weather system
  Weather_init();

  // init settings
  Settings_init();

  // init the messaging thing
  messaging_init(forceScreenRedraw);

  // Create main Window element and assign to pointer
  mainWindow = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(mainWindow, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  bool connected = bluetooth_connection_service_peek();
  bluetoothStateChanged(connected);
  bluetooth_connection_service_subscribe(bluetoothStateChanged);

  // register with battery service
  battery_state_service_subscribe(batteryStateChanged);
}

static void deinit() {
  // Destroy Window
  window_destroy(mainWindow);

  // unload weather stuff
  Weather_deinit();
  Settings_deinit();

  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
