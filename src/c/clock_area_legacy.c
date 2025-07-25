#ifdef PBL_PLATFORM_APLITE
#include <pebble.h>
#include "clock_area.h"
#include "settings.h"
#include "clock_digit_legacy.h"

#define DIGIT_COUNT 4

static Layer* clock_area_layer;
static ClockDigit digits[DIGIT_COUNT];

static char time_hours[3];
static char time_minutes[3];

static GPoint digit_points[DIGIT_COUNT];
static GRect screen_rect;

void ClockArea_update_time(struct tm* time_info) {
  if (clock_is_24h_style()) {
    strftime(time_hours, sizeof(time_hours), globalSettings.showLeadingZero ? "%H" : "%k", time_info);
  } else {
    strftime(time_hours, sizeof(time_hours), globalSettings.showLeadingZero ? "%I" : "%l", time_info);
  }

  strftime(time_minutes, sizeof(time_minutes), "%M", time_info);
}

void ClockArea_redraw() {
  GColor fg = globalSettings.timeColor;
  GColor bg = globalSettings.timeBgColor;
  int offset = globalSettings.sidebarOnLeft ? 30 : 0;

  int h_font = globalSettings.clockFontId;
  int m_font = globalSettings.clockFontId;

  if (h_font == FONT_SETTING_BOLD_H) {
    h_font = FONT_SETTING_BOLD;
    m_font = FONT_SETTING_DEFAULT;
  } else if (m_font == FONT_SETTING_BOLD_M) {
    h_font = FONT_SETTING_DEFAULT;
    m_font = FONT_SETTING_BOLD;
  }

  // H1
  if (time_hours[0] == ' ' && !globalSettings.showLeadingZero) {
    ClockDigit_setBlank(&digits[0]);
  } else {
    ClockDigit_setNumber(&digits[0], time_hours[0] - '0', h_font);
    ClockDigit_setColor(&digits[0], fg, bg);
    ClockDigit_offsetPosition(&digits[0], offset);
  }

  // H2
  ClockDigit_setNumber(&digits[1], time_hours[1] - '0', h_font);
  ClockDigit_setColor(&digits[1], fg, bg);
  ClockDigit_offsetPosition(&digits[1], offset);

  // M1
  ClockDigit_setNumber(&digits[2], time_minutes[0] - '0', m_font);
  ClockDigit_setColor(&digits[2], fg, bg);
  ClockDigit_offsetPosition(&digits[2], offset);

  // M2
  ClockDigit_setNumber(&digits[3], time_minutes[1] - '0', m_font);
  ClockDigit_setColor(&digits[3], fg, bg);
  ClockDigit_offsetPosition(&digits[3], offset);
}

void ClockArea_init(Window* window) {
  screen_rect = layer_get_bounds(window_get_root_layer(window));

  // Set positions for digits (must be done at runtime)
  #ifdef PBL_ROUND
    digit_points[0] = GPoint(40, 17);
    digit_points[1] = GPoint(90, 17);
    digit_points[2] = GPoint(40, 92);
    digit_points[3] = GPoint(90, 92);
  #else
    digit_points[0] = GPoint(7, 7);
    digit_points[1] = GPoint(60, 7);
    digit_points[2] = GPoint(7, 90);
    digit_points[3] = GPoint(60, 90);
  #endif

  clock_area_layer = layer_create(screen_rect);
  layer_add_child(window_get_root_layer(window), clock_area_layer);

  for (int i = 0; i < DIGIT_COUNT; i++) {
    ClockDigit_construct(&digits[i], digit_points[i]);
    layer_add_child(clock_area_layer, bitmap_layer_get_layer(digits[i].imageLayer));
  }

  window_set_background_color(window, globalSettings.timeBgColor);
}

void ClockArea_deinit() {
  for (int i = 0; i < DIGIT_COUNT; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(digits[i].imageLayer));
    ClockDigit_destruct(&digits[i]);
  }

  layer_destroy(clock_area_layer);
}
#endif