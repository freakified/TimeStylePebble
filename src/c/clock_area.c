#ifndef PBL_PLATFORM_APLITE
#include <pebble.h>

#include "clock_area.h"
#include "settings.h"
#include "sidebar.h"

#include "fctx/fctx.h"
#include "fctx/ffont.h"

#define ROUND_VERTICAL_PADDING 15
#define ROUND_OBSTRUCTED_PADDING 4

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
  [FACE_GARAMOND] = { RESOURCE_ID_GARAMOND_REGULAR_FFONT, RESOURCE_ID_GARAMOND_BOLD_FFONT },
};

static FFont* faces[FACE_COUNT][2];

GRect screen_rect;

// "private" functions

// the horizontal ink extent of a string, in the same units (and from the
// same origin) fctx_draw_string uses with left alignment. fctx centers by
// advance width, but digit side bearings are lopsided (LECO's overhang
// their right edge, Avenir's "1" sits well right of its box), which
// pushes the time visibly off center (most of all on round watches)
// (the strings here are all ASCII digits and spaces, so no utf8 decoding)
static void string_ink_extent(const char* text, FFont* font, fixed_t* left, fixed_t* right) {
  fixed_t pen = 0;
  *left = INT32_MAX;
  *right = INT32_MIN;

  for(const char* p = text; *p; p++) {
    FGlyph* glyph = ffont_glyph_info(font, (uint8_t)*p);
    if(!glyph) {
      continue;
    }

    void* data = ffont_glyph_outline(font, glyph);
    void* end = data + glyph->path_data_length;

    while(data < end) {
      FPathDrawCommand* cmd = data;
      fixed16_t* param = cmd->params;
      int x_count = 0;  // x params come first in each point pair
      int pairs = 0;

      switch(cmd->code) {
        case 'M': case 'L': case 'T': pairs = 1; break;
        case 'S': case 'Q':           pairs = 2; break;
        case 'C':                     pairs = 3; break;
        case 'H':                     x_count = 1; break;
        case 'V':                     param++; break;
        case 'Z':                     break;
        default:                      return;  // fctx would bail too
      }

      // every curve point stays inside its control polygon, so the
      // control points bound the outline (tightly, for well-formed
      // fonts, which put on-curve points at the extremes)
      for(int i = 0; i < pairs + x_count; i++) {
        fixed_t x = pen + *param;
        param += pairs ? 2 : 1;
        if(x < *left)  *left = x;
        if(x > *right) *right = x;
      }

      data = param;
    }

    pen += glyph->horiz_adv_x;
  }

  if(*left > *right) {
    *left = *right = 0;
  }
}

// draws text with its ink, not its advance box, centered on center_x.
// leading blanks (%k, %l) are measured as "0"s, so a single-digit hour
// keeps the spot it would have in a two-digit one rather than centering
static void draw_string_ink_centered(FContext* fctx, const char* text, FFont* font,
                                     fixed_t center_x, fixed_t y, FTextAnchor anchor) {
  char measured[4];
  strncpy(measured, text, sizeof(measured) - 1);
  measured[sizeof(measured) - 1] = '\0';

  // the blanks themselves aren't drawn; the digits after them start
  // where they would behind that many "0"s
  fixed_t lead = 0;
  FGlyph* zero = ffont_glyph_info(font, '0');
  for(char* p = measured; *p == ' ' && zero; p++, text++) {
    *p = '0';
    lead += zero->horiz_adv_x;
  }

  fixed_t left, right;
  string_ink_extent(measured, font, &left, &right);

  // font units -> screen, the same scaling fctx_string_width applies
  fixed_t x = (lead - (left + right) / 2) * fctx->transform_scale_to.x / fctx->transform_scale_from.x;

  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, FPoint(center_x + x, y));
  fctx_draw_string(fctx, text, font, GTextAlignmentLeft, anchor);
  fctx_end_fill(fctx);
}

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
    // the sidebar stays put under timeline quick view; only the time
    // shrinks to fit above it. the obstruction's top edge is a wide chord
    // of the circle, so it needs much less padding than the screen's edge.
    // a bottom bar widget takes the place of the bottom padding
    int bottomPadding = Sidebar_reservedBottom();
    if (bottomPadding < ROUND_VERTICAL_PADDING) {
      bottomPadding = ROUND_VERTICAL_PADDING;
    }
    int bottom = screen_rect.size.h - bottomPadding;
    int obstructedBottom = bounds.origin.y + bounds.size.h - ROUND_OBSTRUCTED_PADDING;

    if (bottom > obstructedBottom) {
      bottom = obstructedBottom;
    }

    bounds = GRect(0, ROUND_VERTICAL_PADDING, screen_rect.size.w, bottom - ROUND_VERTICAL_PADDING);
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
  }

  #ifndef PBL_ROUND
    // for rectangular watches, adjust X position based on sidebar position
    if(Sidebar_verticalEdge() == REGION_EDGE_LEFT) {
      h_adjust += Sidebar_verticalThickness() / 2;
    } else {
      h_adjust -= Sidebar_verticalThickness() / 2;
    }
  #endif

  // each string gets its own fill so the AA renderer's transient
  // coverage buffer only ever spans one line of digits
  fixed_t center_x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);

  fctx_set_text_em_height(&fctx, hours_font, font_size);
  draw_string_ink_centered(&fctx, time_hours, hours_font, center_x,
                           INT_TO_FIXED(bounds.origin.y + v_padding + v_adjust), FTextAnchorTop);

  fctx_set_text_em_height(&fctx, minutes_font, font_size);
  draw_string_ink_centered(&fctx, time_minutes, minutes_font, center_x,
                           INT_TO_FIXED(bounds.origin.y + bounds.size.h - v_padding + v_adjust), FTextAnchorBaseline);

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
