#include <pebble.h>
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

// the four digits on the clock, ordered h1 h2, m1 m2
static ClockDigit clockDigits[4];

void sidebarLayerUpdateProc(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, mainAreaFG);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
}

static void main_window_load(Window *window) {
  ClockDigit_construct(&clockDigits[0], GPoint(7, 7));
  ClockDigit_construct(&clockDigits[1], GPoint(60, 7));
  ClockDigit_construct(&clockDigits[2], GPoint(7, 90));
  ClockDigit_construct(&clockDigits[3], GPoint(60, 90));
  
  for(int i = 0; i < 4; i++) {
    ClockDigit_setNumber(&clockDigits[i], i+1);
    ClockDigit_setColor(&clockDigits[i], mainAreaFG, mainAreaBG);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clockDigits[i].imageLayer));
  }
  
  // init the sidebar layer
  sidebarLayer = layer_create(GRect(114, 0, 30, 168));
  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, sidebarLayerUpdateProc);

  window_set_background_color(window, mainAreaBG);
  
  // Make sure the time is displayed from the start
//   update_clock();
}

static void main_window_unload(Window *window) {
  for(int i = 0; i < 4; i++) {
    ClockDigit_destruct(&clockDigits[i]);
  }
  
  layer_destroy(sidebarLayer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
//   update_clock();
}

static void init() {
  mainAreaFG = GColorOrange;
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
  //tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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