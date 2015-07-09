#include <pebble.h>
#include "messaging.h"
#include "bgpicker.h"

// #define FORCE_BACKLIGHT
#define FORCE_12H true
#define TIME_STR_LEN 6
#define AMPM_STR_LEN 9
#define DATE_STR_LEN 25

// windows and layers
static Window* mainWindow;
static TextLayer* timeLayer;
static TextLayer* dateLayer;
static TextLayer* ampmLayer;

// fonts
static GFont timeFont;
static GFont dateFont;
static GFont ampmFont;

// long-lived strings
static char timeText[TIME_STR_LEN];
static char ampmText[AMPM_STR_LEN];
static char dateText[DATE_STR_LEN]; 

static void update_clock() {
  time_t rawTime;
  struct tm* timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);
  
  // set time string
  if(clock_is_24h_style() && !FORCE_12H) {
    // use 24 hour format
    strftime(timeText, TIME_STR_LEN, "%H:%M", timeInfo);
  } else {
    // use 12 hour format
    strftime(timeText, TIME_STR_LEN, "%I:%M", timeInfo);
    
    // display the am/pm state
    strftime(ampmText, AMPM_STR_LEN, "%p", timeInfo);

    text_layer_set_text(ampmLayer, ampmText);
  }
    
  //now trim leading 0's
  if(timeText[0] == '0') {
    //shuffle everyone back by 1
    for(int i = 0; i < TIME_STR_LEN; i++) {
      timeText[i] = timeText[i + 1];
    }
        
    // move the ampm layer to adjust for the smaller text
    layer_set_frame(text_layer_get_layer(ampmLayer), GRect(95, 83, 60, 40));
  } else {
    // adjust the position of the am/pm layer
    layer_set_frame(text_layer_get_layer(ampmLayer), GRect(117, 83, 60, 40));
  }

  // display this time on the TextLayer
  text_layer_set_text(timeLayer, timeText);
  
  // display the date
  strftime(dateText, DATE_STR_LEN, "%A, %B%e", timeInfo);
//   strncpy(dateText, "Wednesday, September 21", DATE_STR_LEN);
  
  text_layer_set_text(dateLayer, dateText);
  
  // forces the the background image to update, reflecting changes immediately
  window_set_background_color(mainWindow, bgpicker_getCurrentBg(timeInfo));
}

static void main_window_load(Window *window) {
  // create fonts
  timeFont = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  dateFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
//   ampmFont = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
  ampmFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // Create time TextLayer
//   timeLayer = text_layer_create(GRect(7, 34, 130, 50));
  timeLayer = text_layer_create(GRect(5, 59, 130, 50));
  text_layer_set_background_color(timeLayer, GColorClear);
  text_layer_set_text_color(timeLayer, GColorBlack);
  text_layer_set_font(timeLayer, timeFont);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(timeLayer));
  
  // Create date TextLayer
  dateLayer = text_layer_create(GRect(7, 99, 130, 60));
  text_layer_set_background_color(dateLayer, GColorClear);
  text_layer_set_text_color(dateLayer, GColorBlack);
  text_layer_set_overflow_mode(dateLayer, GTextOverflowModeWordWrap);
  text_layer_set_font(dateLayer, dateFont);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(dateLayer));
  
  // Create ampm TextLayer
  ampmLayer = text_layer_create(GRect(117, 83, 60, 40));
  text_layer_set_background_color(ampmLayer, GColorClear);
  text_layer_set_text_color(ampmLayer, GColorBlack);
  text_layer_set_font(ampmLayer, ampmFont);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(ampmLayer));
  
  // Make sure the time is displayed from the start
  update_clock();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(timeLayer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_clock();
}

  
static void init() {
  #ifdef FORCE_BACKLIGHT
  light_enable(true);
  #endif
  
  // load background images
  bgpicker_init();
  
  // init the messaging thing
  messaging_init();
  
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
}

static void deinit() {
  // Destroy Window
  window_destroy(mainWindow);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}