#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "clock_digit.h"
#include "weather.h"
#include "messaging.h"
#include "settings.h"

#define FORCE_BACKLIGHT false
#define FORCE_12H true
  
// windows and layers
static Window* mainWindow;
static Layer* sidebarLayer;

// PDC images
static GDrawCommandImage *dateImage;
static GDrawCommandImage *disconnectImage;

// current bluetooth state
static bool isPhoneConnected;

// fonts
static GFont sidebarFont;
static GFont dateFont;

// the four digits on the clock, ordered h1 h2, m1 m2
static ClockDigit clockDigits[4];

// the date and weather strings
static char currentDayName[4];
static char currentDayNum[3];
static char currentMonth[4];

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
  if(hour / 10 != 0) {
    ClockDigit_setNumber(&clockDigits[0], hour / 10);
  } else {
    ClockDigit_setBlank(&clockDigits[0]);
  }
  
  ClockDigit_setNumber(&clockDigits[1], hour % 10);
  ClockDigit_setNumber(&clockDigits[2], timeInfo->tm_min  / 10);
  ClockDigit_setNumber(&clockDigits[3], timeInfo->tm_min  % 10);
  
  // set all the date strings
  strftime(currentDayName, 4, "%a", timeInfo);
  strftime(currentDayNum,  3, "%e", timeInfo);
  strftime(currentMonth,   4, "%b", timeInfo);
  
  // convert day and month names to uppercase
  for(int i = 0; i < 4; i++) {
    currentDayName[i] = toupper((int)currentDayName[i]);
    currentMonth[i]   = toupper((int)currentMonth[i]);
  }
  
  // remove padding on date num, if needed
  if(currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }
  
  layer_mark_dirty(sidebarLayer);
}

void sidebarLayerUpdateProc(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  
  if (Weather_currentWeatherIcon) {
    gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(2, 7));
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
  
  // now draw in the text
  graphics_draw_text(ctx,
                     currentDayName,
                     sidebarFont,
                     GRect(0, 95, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
  
  if (disconnectImage && !isPhoneConnected) {
    gdraw_command_image_draw(ctx, disconnectImage, GPoint(2, 60));
  }
  
  if (dateImage) {
    gdraw_command_image_draw(ctx, dateImage, GPoint(2, 118));
  }
  
  graphics_draw_text(ctx,
                     currentDayNum,
                     dateFont,
                     GRect(4, 121, 22, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
  
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
  
  redrawSidebar();
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
  dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
  disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
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
 
  gdraw_command_image_destroy(dateImage);
  gdraw_command_image_destroy(disconnectImage);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // every 30 minutes, request new weather data 
  if(tick_time->tm_min % 30 == 0) {
    messaging_requestNewWeatherData();
  }
  
  update_clock();
}



void bluetooth_connection_callback(bool connection){
  isPhoneConnected = connection;
  redrawSidebar();
}


static void init() {
  setlocale(LC_TIME, "fr_FR");
  
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
  bluetooth_connection_callback(connected);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
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