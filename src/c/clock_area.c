#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#include "clock_area.h"
#include "settings.h"

Layer* clock_area_layer;
FFont* clock_font;

// "private" functions
void update_clock_area_layer(Layer *l, GContext* ctx) {
  // fctx -- will it blend?
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, GColorWhite);

  /* Draw the time. */
  FPoint date_pos;
  date_pos.x = INT_TO_FIXED(57);
  date_pos.y = INT_TO_FIXED(160);
  fctx_begin_fill(&fctx);
  fctx_set_text_size(&fctx, clock_font, 97);
  fctx_set_offset(&fctx, date_pos);
  fctx_draw_string(&fctx, "05", clock_font, GTextAlignmentCenter, FTextAnchorBaseline);
  fctx_end_fill(&fctx);

  fctx_deinit_context(&fctx);
}

void ClockArea_init(Window* window) {
  // init the clock area layer
  GRect screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds;

  // TODO: add left-side-sidebar support
  bounds = GRect(0, 0, screen_rect.size.w - 30, screen_rect.size.h);

  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);

  // init font
  clock_font = ffont_create_from_resource(RESOURCE_ID_AVENIR_REGULAR_FFONT);
}

void ClockArea_deinit() {
  layer_destroy(clock_area_layer);
  ffont_destroy(clock_font);
}

void ClockArea_redraw() {
  layer_mark_dirty(clock_area_layer);  
}

void ClockArea_update_time(struct tm* time_info) {
  int hour = time_info->tm_hour;
  int min  = time_info->tm_min;

  if (!clock_is_24h_style()) {
    if(hour > 12) {
      hour %= 12;
    } else if(time_info->tm_hour == 0) {
      hour = 12;
    }
  }
}