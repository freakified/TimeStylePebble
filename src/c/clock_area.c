#ifndef PBL_PLATFORM_APLITE
#include <pebble.h>

#include "clock_area.h"
#include "settings.h"
#include "sidebar.h"

#include "fctx/fctx.h"
#include "fctx/ffont.h"

#define ROUND_VERTICAL_PADDING 15

char time_hours[3];
char time_minutes[3];

Layer* clock_area_layer;
FFont* hours_font;
FFont* minutes_font;

// every face, indexed [typeface][bold]; only the ones the current
// settings use are kept in memory
typedef enum { FACE_AVENIR, FACE_LECO, FACE_GARAMOND, FACE_COUNT } ClockFace;

static const uint32_t face_resources[FACE_COUNT][2] = {
  [FACE_AVENIR]   = { RESOURCE_ID_AVENIR_REGULAR_FFONT, RESOURCE_ID_AVENIR_BOLD_FFONT },
  [FACE_LECO]     = { RESOURCE_ID_LECO_LIGHT_FFONT,     RESOURCE_ID_LECO_REGULAR_FFONT },
  [FACE_GARAMOND] = { RESOURCE_ID_GARAMOND_MEDIUM_FFONT, RESOURCE_ID_GARAMOND_BOLD_FFONT },
};

static FFont* faces[FACE_COUNT][2];

GRect screen_rect;

// "private" functions
static ClockFace current_face() {
  switch(settings.clockFontId) {
    case FONT_SETTING_LECO:     return FACE_LECO;
    case FONT_SETTING_GARAMOND: return FACE_GARAMOND;
    default:                    return FACE_AVENIR;
  }
}

static void set_face_loaded(ClockFace face, bool bold, bool needed) {
  FFont** font = &faces[face][bold];

  if(needed && *font == NULL) {
    *font = ffont_create_from_resource(face_resources[face][bold]);
  } else if(!needed && *font != NULL) {
    ffont_destroy(*font);
    *font = NULL;
  }
}

void update_fonts() {
  ClockFace face = current_face();

  // free what's no longer used before loading anything new, so a switch
  // never holds both the old and new faces at once
  for(int f = 0; f < FACE_COUNT; f++) {
    for(int bold = 0; bold < 2; bold++) {
      bool needed = f == (int)face &&
                    (settings.hoursBold == bold || settings.minutesBold == bold);
      if(!needed) {
        set_face_loaded(f, bold, false);
      }
    }
  }

  set_face_loaded(face, settings.hoursBold, true);
  set_face_loaded(face, settings.minutesBold, true);

  hours_font = faces[face][settings.hoursBold];
  minutes_font = faces[face][settings.minutesBold];
}

void update_clock_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  GRect bounds = layer_get_unobstructed_bounds(l);

  #ifdef PBL_ROUND
    bounds = GRect(0, ROUND_VERTICAL_PADDING, screen_rect.size.w, screen_rect.size.h - ROUND_VERTICAL_PADDING * 2);
  #else
    // reserve space for top/bottom sidebar regions, combining with any
    // system obstruction (timeline quick view) already in the bounds
    int top = bounds.origin.y + Sidebar_reservedTop();
    int bottom = bounds.origin.y + bounds.size.h;
    int bottomLimit = screen_rect.size.h - Sidebar_reservedBottom();

    if (bottom > bottomLimit) {
      bottom = bottomLimit;
    }

    bounds = GRect(bounds.origin.x, top, bounds.size.w, bottom - top);
  #endif

  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  #ifdef PBL_COLOR
    // leco looks awful with antialiasing; this must be set BEFORE
    // fctx_init_context, which is one of the functions the toggle swaps
    fctx_enable_aa(settings.clockFontId != FONT_SETTING_LECO);
  #endif

  FContext fctx;
  fctx_init_context(&fctx, ctx);

  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, settings.timeColor);


  // calculate font size
  int font_size = 4 * bounds.size.h / 7;

  // avenir + avenir bold metrics
  int v_padding = bounds.size.h / 16;
  int h_adjust = 0;
  int v_adjust = 0;

  // alternate metrics for LECO
  if(settings.clockFontId == FONT_SETTING_LECO) {
    font_size = 4 * bounds.size.h / 7 + 6;
    v_padding = bounds.size.h / 20;
    h_adjust = -4;
    v_adjust = 0;
  }

  // if it's a round watch, EVERYTHING CHANGES
  #ifdef PBL_ROUND
    // the round inset is already bounds.origin.y; adding it here too
    // pushed the time ROUND_VERTICAL_PADDING below center

    if(settings.clockFontId != FONT_SETTING_LECO) {
      h_adjust = -1;
    }
  #else
    // for rectangular watches, adjust X position based on sidebar position
    if(Sidebar_verticalEdge() == REGION_EDGE_LEFT) {
      h_adjust += Sidebar_verticalThickness() / 2;
    } else {
      h_adjust -= Sidebar_verticalThickness() / 2 + 1;
    }
  #endif

  // each string gets its own fill so the AA renderer's transient
  // coverage buffer only ever spans one line of digits
  FPoint time_pos;
  fctx_set_text_em_height(&fctx, hours_font, font_size);
  fctx_set_text_em_height(&fctx, minutes_font, font_size);

  // draw hours
  time_pos.x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);
  time_pos.y = INT_TO_FIXED(bounds.origin.y + v_padding + v_adjust);
  fctx_begin_fill(&fctx);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_hours, hours_font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(&fctx);

  //draw minutes
  time_pos.y = INT_TO_FIXED(bounds.origin.y + bounds.size.h - v_padding + v_adjust);
  fctx_begin_fill(&fctx);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_minutes, minutes_font, GTextAlignmentCenter, FTextAnchorBaseline);
  fctx_end_fill(&fctx);

  fctx_deinit_context(&fctx);
}


void ClockArea_init(Window* window) {
  // record the screen size, since we NEVER GET IT AGAIN
  screen_rect = layer_get_bounds(window_get_root_layer(window));

  GRect bounds;
  bounds = GRect(0, 0, screen_rect.size.w, screen_rect.size.h);

  // init the clock area layer
  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);

  // load and select fonts based on settings
  update_fonts();
}

void ClockArea_deinit() {
  layer_destroy(clock_area_layer);

  for(int f = 0; f < FACE_COUNT; f++) {
    set_face_loaded(f, false, false);
    set_face_loaded(f, true, false);
  }
}

void ClockArea_redraw() {
  // check if the fonts need to be switched
  update_fonts();

  layer_mark_dirty(clock_area_layer);
}

void ClockArea_update_time(struct tm* time_info) {

  // hours
  if (clock_is_24h_style()) {
    strftime(time_hours, sizeof(time_hours), (settings.showLeadingZero) ? "%H" : "%k", time_info);
  } else {
    strftime(time_hours, sizeof(time_hours), (settings.showLeadingZero) ? "%I" : "%l", time_info);
  }

  // minutes
  strftime(time_minutes, sizeof(time_minutes), "%M", time_info);
}

#endif
