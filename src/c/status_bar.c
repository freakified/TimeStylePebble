#include "status_bar.h"
#include "settings.h"
#include "sidebar.h"
#include "sidebar_layout.h"
#include <pebble.h>

/*
 * The status bar is a single tiny prerendered icon that overlays the
 * top of the watchface without reserving any space -- the clock keeps
 * its full size, and the icon sits in the clock's natural top
 * padding. On rectangular watches it sits at the top corner opposite
 * the vertical sidebar; on round watches it centers at the top.
 *
 * Disconnection always wins, then quiet time, then (only if the
 * "always show connection" setting is on) a "connected" icon;
 * otherwise nothing is drawn.
 */

#define STATUS_BAR_HEIGHT 15
#define STATUS_BAR_PADDING_X 3

// round watches: the icon centers on this row
#define ROUND_STATUS_BAR_HEIGHT 22
#define ROUND_STATUS_CENTER_Y 11

static GRect screen_rect;
static Layer *statusBarLayer;

static GBitmap *dndIcon;
static GBitmap *connectedIcon;
static GBitmap *disconnectedIcon;

// the icon art is a single opaque color on transparency; repaint the
// opaque palette entries so the icons follow the configured color
static void recolor_icon(GBitmap *icon, GColor color) {
  GColor *palette = gbitmap_get_palette(icon);

  if (palette == NULL) {
    return;
  }

  int paletteSize;

  switch (gbitmap_get_format(icon)) {
    case GBitmapFormat1BitPalette: paletteSize = 2;  break;
    case GBitmapFormat2BitPalette: paletteSize = 4;  break;
    case GBitmapFormat4BitPalette: paletteSize = 16; break;
    default: return;
  }

  for (int i = 0; i < paletteSize; i++) {
    if (palette[i].a != 0) {
      palette[i] = color;
    }
  }
}

// the bar draws directly on the watchface background, matching the time
// color; the palettes are only rewritten when that color changes
static GColor iconColor;

static void update_icon_color(void) {
  if (gcolor_equal(iconColor, settings.timeColor)) {
    return;
  }

  iconColor = settings.timeColor;
  recolor_icon(dndIcon, iconColor);
  recolor_icon(connectedIcon, iconColor);
  recolor_icon(disconnectedIcon, iconColor);
}

static void draw_icon(GContext *ctx, GBitmap *icon, int x, int centerY) {
  GSize size = gbitmap_get_bounds(icon).size;
  GRect iconRect = GRect(x, centerY - size.h / 2, size.w, size.h);

  // knock out a 1px background margin so the icon stays legible where
  // large clock fonts reach up into it
  graphics_context_set_fill_color(ctx, settings.timeBgColor);
  graphics_fill_rect(ctx, grect_inset(iconRect, GEdgeInsets(-1)), 0,
                     GCornerNone);

  graphics_draw_bitmap_in_rect(ctx, icon, iconRect);
}

static void status_bar_update(Layer *l, GContext *ctx) {
  if (!settings.enableStatusBar) {
    return;
  }

  GRect bounds = layer_get_bounds(l);

  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  bool connected = bluetooth_connection_service_peek();

  GBitmap *statusIcon = NULL;

  if (!connected) {
    statusIcon = disconnectedIcon;
  } else if (quiet_time_is_active()) {
    statusIcon = dndIcon;
  } else if (settings.statusShowConnection) {
    statusIcon = connectedIcon;
  }

  if (statusIcon == NULL) {
    return;
  }

#ifdef PBL_ROUND
  int iconWidth = gbitmap_get_bounds(statusIcon).size.w;
  draw_icon(ctx, statusIcon, (bounds.size.w - iconWidth) / 2,
            ROUND_STATUS_CENTER_Y);
#else
  draw_icon(ctx, statusIcon, STATUS_BAR_PADDING_X, bounds.size.h / 2);
#endif
}

#ifndef PBL_ROUND
// the bar yields its corner to the vertical sidebar, like the bottom bar
static GRect statusbar_frame(void) {
  int reserved = (Sidebar_verticalEdge() == REGION_EDGE_NONE)
                     ? 0
                     : Sidebar_verticalThickness() + HBAR_VBAR_GAP;
  int x = (Sidebar_verticalEdge() == REGION_EDGE_LEFT) ? reserved : 0;
  int width = screen_rect.size.w - reserved;

  if (width < 0) {
    width = 0;
  }

  return GRect(x, 0, width, STATUS_BAR_HEIGHT);
}
#endif

void StatusBar_init(Window *window) {
  Layer *root = window_get_root_layer(window);
  screen_rect = layer_get_bounds(root);

  dndIcon = gbitmap_create_with_resource(RESOURCE_ID_STATUS_DND);
  connectedIcon = gbitmap_create_with_resource(RESOURCE_ID_STATUS_CONNECTED);
  disconnectedIcon =
      gbitmap_create_with_resource(RESOURCE_ID_STATUS_DISCONNECTED);
  iconColor = GColorClear; // fresh bitmaps; the time is never clear
  update_icon_color();

#ifdef PBL_ROUND
  statusBarLayer = layer_create(
      GRect(0, 0, screen_rect.size.w, ROUND_STATUS_BAR_HEIGHT));
#else
  statusBarLayer = layer_create(statusbar_frame());
#endif

  layer_set_update_proc(statusBarLayer, status_bar_update);
  layer_add_child(root, statusBarLayer);
  layer_set_hidden(statusBarLayer, !settings.enableStatusBar);
}

void StatusBar_deinit(void) {
  layer_destroy(statusBarLayer);

  gbitmap_destroy(dndIcon);
  gbitmap_destroy(connectedIcon);
  gbitmap_destroy(disconnectedIcon);
}

void StatusBar_redraw(void) {
  update_icon_color();

#ifndef PBL_ROUND
  layer_set_frame(statusBarLayer, statusbar_frame());
#endif

  layer_set_hidden(statusBarLayer, !settings.enableStatusBar);
  layer_mark_dirty(statusBarLayer);
}
