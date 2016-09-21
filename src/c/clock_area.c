#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#include "clock_area.h"
#include "settings.h"

char time_hours[3];
char time_minutes[3];

Layer* clock_area_layer;
FFont* clock_font;

// "private" functions
void update_clock_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  GRect bounds = layer_get_unobstructed_bounds(l);
  
  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, globalSettings.timeColor);
  
  // draw the time
  int font_size = 4 * bounds.size.h / 7;

  // avenir
  int v_padding = bounds.size.h / 16;
  int h_adjust = 0;

  // alternate params for LECO
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    v_padding = bounds.size.h / 13;
    h_adjust = -2;

    #ifdef PBL_COLOR
      fctx_enable_aa(false);
    #endif
  }

  FPoint time_pos;
  fctx_begin_fill(&fctx);
  fctx_set_text_size(&fctx, clock_font, font_size);

  // draw hours
  time_pos.x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);
  time_pos.y = INT_TO_FIXED(v_padding);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_hours, clock_font, GTextAlignmentCenter, FTextAnchorTop);

  //draw minutes 
  time_pos.y = INT_TO_FIXED(bounds.size.h - v_padding);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_minutes, clock_font, GTextAlignmentCenter, FTextAnchorBaseline);
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
  // clock_font = ffont_create_from_resource(RESOURCE_ID_LECO_REGULAR_FFONT);
  // clock_font = ffont_create_from_resource(RESOURCE_ID_CONSOLAS_REGULAR_FFONT);
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
  // hours
  if (clock_is_24h_style()) {
    strftime(time_hours, sizeof(time_hours), "%H", time_info);
  } else {
    strftime(time_hours, sizeof(time_hours), "%I", time_info);
  }

  // minutes
  strftime(time_minutes, sizeof(time_minutes), "%M", time_info);
}