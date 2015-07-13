#include <pebble.h>
#include <ctype.h>
#include "clock_digit.h"
// #include "messaging.h"
// #include "bgpicker.h"

// #define FORCE_BACKLIGHT true
#define FORCE_BACKLIGHT false
#define FORCE_12H true
  
static GColor mainAreaFG;
static GColor mainAreaBG;

// windows and layers
static Window* mainWindow;
static Layer* sidebarLayer;

// PDC images
static GDrawCommandImage *weatherImage;
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
  // set the locale
  setlocale(LC_TIME, "es_ES");
  
  time_t rawTime;
  struct tm* timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);
  
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
    ClockDigit_setNumber(&clockDigits[0], 10);
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
  graphics_context_set_fill_color(ctx, mainAreaFG);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  
  if (weatherImage) {
    gdraw_command_image_draw(ctx, weatherImage, GPoint(2, 7));
  }
  
  // draw weather data
  graphics_draw_text(ctx,
                     " 74°",
                     sidebarFont,
                     GRect(-5, 30, 38, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
  
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

static void main_window_load(Window *window) {
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "trying to construct");
  ClockDigit_construct(&clockDigits[0], GPoint(7, 7));
  ClockDigit_construct(&clockDigits[1], GPoint(60, 7));
  ClockDigit_construct(&clockDigits[2], GPoint(7, 90));
  ClockDigit_construct(&clockDigits[3], GPoint(60, 90));
  
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Made it past construction");
  
  for(int i = 0; i < 4; i++) {
    ClockDigit_setColor(&clockDigits[i], mainAreaFG, mainAreaBG);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clockDigits[i].imageLayer));
  }
  
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Made it past color setting");
  
  // load font
  sidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  dateFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // init the sidebar layer
  sidebarLayer = layer_create(GRect(114, 0, 30, 168));
  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, sidebarLayerUpdateProc);
  
  // temp: load the weather PDC image
  weatherImage = gdraw_command_image_create_with_resource(RESOURCE_ID_WEATHER_THUNDERSTORM);
  dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
  disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
  if (!weatherImage && !dateImage) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load a PDC image.");
  }
  
  window_set_background_color(window, mainAreaBG);
  
  // Make sure the time is displayed from the start
  update_clock();
}

static void main_window_unload(Window *window) {
  for(int i = 0; i < 4; i++) {
    ClockDigit_destruct(&clockDigits[i]);
  }
  
  layer_destroy(sidebarLayer);
 
  gdraw_command_image_destroy(weatherImage);
  gdraw_command_image_destroy(dateImage);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_clock();
}

void bluetooth_connection_callback(bool connection){
   isPhoneConnected = connection;
   layer_mark_dirty(sidebarLayer);
}

static void init() {
  setlocale(LC_TIME, "fr_FR");
  
  mainAreaFG = GColorOrange;
//   mainAreaFG = GColorGreen;
  mainAreaBG = GColorBlack;
  
  if(FORCE_BACKLIGHT) {
    light_enable(true);
  }
  
  // load background images
  //bgpicker_init();
  
  // init the messaging thing
  //messaging_init();
  
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
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}