#include <pebble.h>
#include "clock_area.h"
#include "messaging.h"
#include "settings.h"
#include "weather.h"
#include "sidebar.h"
#include "status_bar.h"
#include "util.h"

// windows and layers
static Window* mainWindow;
static Layer* windowLayer;

// current bluetooth state
static bool isPhoneConnected;

// current time service subscription
static bool updatingEverySecond;

// try to randomize when watches call the weather API (0-29; the phone is
// pinged at this minute and 30 past it)
static uint8_t weatherRefreshMinute;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

static void update_clock() {
  time_t rawTime;
  struct tm* timeInfo;

#ifdef USE_FAKE_TIME
  struct tm fakeTime = {
    .tm_hour = 6,
    .tm_min  = 23,
    .tm_sec  = 0,
    .tm_mday = 24,
    .tm_mon  = 3,
    .tm_year = 126,
    .tm_wday = 5,
  };
  timeInfo = &fakeTime;
#else
  time(&rawTime);
  timeInfo = localtime(&rawTime);
#endif

  ClockArea_update_time(timeInfo);
  Sidebar_updateTime(timeInfo);
}

/* forces everything on screen to be redrawn -- perfect for keeping track of settings! */
static void redrawScreen() {

  // check if the tick handler frequency should be changed
  if(dynamicSettings.updateScreenEverySecond != updatingEverySecond) {
    tick_timer_service_unsubscribe();

    if(dynamicSettings.updateScreenEverySecond) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      updatingEverySecond = true;
    } else {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      updatingEverySecond = false;
    }
  }

  window_set_background_color(mainWindow, settings.timeBgColor);

  // maybe the language changed!
  update_clock();

  // update the sidebar
  Sidebar_redraw();

  StatusBar_redraw();

  ClockArea_redraw();
}

static void main_window_load(Window *window) {
  window_set_background_color(window, settings.timeBgColor);

  // create the sidebar
  Sidebar_init(window);

  ClockArea_init(window);

  StatusBar_init(window);

  // Make sure the time is displayed from the start
  redrawScreen();
}

static void main_window_unload(Window *window) {
  StatusBar_deinit();
  ClockArea_deinit();
  Sidebar_deinit();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // every 30 minutes, ping the phone (weather data, world time DST offsets)
  if(dynamicSettings.enablePhonePolling) {
    if(tick_time->tm_min % 30 == weatherRefreshMinute && tick_time->tm_sec == 0) {
      messaging_requestNewWeatherData();
    }
  }

  // every hour, if requested, vibrate
  if(!quiet_time_is_active() && tick_time->tm_sec == 0) {
    if(settings.hourlyVibe == VIBE_EVERY_HOUR) { // hourly vibes only
      if(tick_time->tm_min == 0) {
        vibes_double_pulse();
      }
    } else if(settings.hourlyVibe == VIBE_EVERY_HALF_HOUR) {  // hourly and half-hourly
      if(tick_time->tm_min == 0) {
        vibes_double_pulse();
      } else if(tick_time->tm_min == 30) {
        vibes_short_pulse();
      }
    }
  }

  update_clock();

  // redraw all screen
  Sidebar_redraw();
  StatusBar_redraw(); // quiet time can toggle between ticks
  ClockArea_redraw();
}

static void bluetoothStateChanged(bool newConnectionState) {
  // if the phone was connected but isn't anymore and the user has opted in,
  // trigger a vibration
  if(!quiet_time_is_active() && isPhoneConnected && !newConnectionState && settings.btVibe) {
    static uint32_t const segments[] = { 200, 100, 100, 100, 500 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
      };
    vibes_enqueue_custom_pattern(pat);
  }

  // if the phone was disconnected and isn't anymore, update the data
  if(!isPhoneConnected && newConnectionState && dynamicSettings.enablePhonePolling) {
    messaging_requestNewWeatherData();
  }

  isPhoneConnected = newConnectionState;

  Sidebar_redraw();
  StatusBar_redraw();
}

// force the sidebar and status bar to redraw any time the battery state changes
static void batteryStateChanged(BatteryChargeState charge_state) {
  Sidebar_redraw();
  StatusBar_redraw();
}

#ifdef PBL_HEALTH
// heart rate readings arrive asynchronously; without this the heart rate
// widget only refreshed on the minute tick, showing stale or no data
static void healthEventHandler(HealthEventType event, void *context) {
  if(event == HealthEventHeartRateUpdate && dynamicSettings.enableHeartRate) {
    Sidebar_redraw();
  }
}
#endif

// fixes for disappearing elements after notifications
// (from http://codecorner.galanter.net/2016/01/08/solved-issue-with-pebble-framebuffer-after-notification-is-dismissed/)
static void app_focus_changing(bool focusing) {
  if (focusing) {
     layer_set_hidden(windowLayer, true);
  }
}

static void app_focus_changed(bool focused) {
  if (focused) {
     layer_set_hidden(windowLayer, false);
     layer_mark_dirty(windowLayer);
  }
}

// timeline quick view: tell the sidebar where the screen is headed so it
// can settle on its widgets up front; the system redraws every frame of
// the animation itself
static void unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
  Sidebar_obstructionWillChange(final_unobstructed_screen_area);
}

static void unobstructed_did_change(void *context) {
  Sidebar_obstructionDidChange();
}

static void init() {
  setlocale(LC_ALL, "");

  srand(time(NULL));

  weatherRefreshMinute = rand() % 30;

  // init settings
  Settings_init();

  // init weather system
  Weather_init();

  // init the messaging thing
  messaging_init(redrawScreen);

  // Create main Window element and assign to pointer
  mainWindow = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(mainWindow, true);

  windowLayer = window_get_root_layer(mainWindow);

  // Register with TickTimerService
  if(dynamicSettings.updateScreenEverySecond) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    updatingEverySecond = true;
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    updatingEverySecond = false;
  }

  // no weather request here: the phone fetches it on its own when the
  // app's JS starts up
  isPhoneConnected = bluetooth_connection_service_peek();
  bluetooth_connection_service_subscribe(bluetoothStateChanged);

  // register with battery service
  battery_state_service_subscribe(batteryStateChanged);

#ifdef PBL_HEALTH
  health_service_events_subscribe(healthEventHandler, NULL);
#endif

  unobstructed_area_service_subscribe((UnobstructedAreaHandlers){
    .will_change = unobstructed_will_change,
    .did_change = unobstructed_did_change
  }, NULL);

  // set up focus change handlers
  app_focus_service_subscribe_handlers((AppFocusHandlers){
    .did_focus = app_focus_changed,
    .will_focus = app_focus_changing
  });
}

static void deinit() {
  // Destroy Window
  window_destroy(mainWindow);

  // unload weather stuff
  Weather_deinit();

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  unobstructed_area_service_unsubscribe();

#ifdef PBL_HEALTH
  health_service_events_unsubscribe();
#endif
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
