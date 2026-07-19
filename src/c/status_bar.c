#include "status_bar.h"
#include "settings.h"
#include "sidebar.h"
#include "sidebar_layout.h"
#include <pebble.h>

/*
 * The status bar is a thin strip of tiny prerendered icons: quiet time
 * on the left, phone connection in the middle, and battery on the
 * right. It overlays the top of the watchface without reserving any
 * space -- the clock keeps its full size, and the icons sit in the
 * clock's natural top padding. On rectangular watches the strip spans
 * the non-sidebar width; on round watches the icons cluster at the top
 * center.
 */

#define STATUS_BAR_HEIGHT 15
#define STATUS_BAR_PADDING_X 3

// round watches: the icon cluster centers on this row
#define ROUND_STATUS_BAR_HEIGHT 22
#define ROUND_STATUS_CENTER_Y 11
#define ROUND_STATUS_ICON_GAP 6

// the charge level fill, inset from the battery outline by a 1px gap
#define STATUS_BATTERY_FILL_X 2
#define STATUS_BATTERY_FILL_Y 2
#define STATUS_BATTERY_FILL_W 10
#define STATUS_BATTERY_FILL_H 4

static GRect screen_rect;
static Layer *statusBarLayer;

static GBitmap *dndIcon;
static GBitmap *batteryIcon;
static GBitmap *batteryChargeIcon;
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

static void draw_icon(GContext *ctx, GBitmap *icon, int x, int centerY) {
  GSize size = gbitmap_get_bounds(icon).size;
  graphics_draw_bitmap_in_rect(
      ctx, icon, GRect(x, centerY - size.h / 2, size.w, size.h));
}

// while charging the outline carries a bolt cutout instead of a level
static void draw_battery(GContext *ctx, BatteryChargeState state, int x,
                         int centerY, GColor color) {
  GBitmap *icon = state.is_charging ? batteryChargeIcon : batteryIcon;
  GSize size = gbitmap_get_bounds(icon).size;
  int y = centerY - size.h / 2;

  graphics_draw_bitmap_in_rect(ctx, icon, GRect(x, y, size.w, size.h));

  if (!state.is_charging) {
    int fillWidth =
        (STATUS_BATTERY_FILL_W * state.charge_percent + 50) / 100;
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx,
                       GRect(x + STATUS_BATTERY_FILL_X,
                             y + STATUS_BATTERY_FILL_Y, fillWidth,
                             STATUS_BATTERY_FILL_H),
                       0, GCornerNone);
  }
}

static void status_bar_update(Layer *l, GContext *ctx) {
  if (!settings.enableStatusBar) {
    return;
  }

  GRect bounds = layer_get_bounds(l);

  // the bar draws directly on the watchface background, so it follows
  // the "dark" region text color
  GColor color = settings.altSidebarTextColor;

  recolor_icon(dndIcon, color);
  recolor_icon(batteryIcon, color);
  recolor_icon(batteryChargeIcon, color);
  recolor_icon(connectedIcon, color);
  recolor_icon(disconnectedIcon, color);

  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  BatteryChargeState battery = battery_state_service_peek();
  bool connected = bluetooth_connection_service_peek();

  // battery and connection always show; quiet time only while active
  bool showDnd = quiet_time_is_active();

  GBitmap *connectionIcon = connected ? connectedIcon : disconnectedIcon;

#ifdef PBL_ROUND
  GBitmap *cluster[3];
  int count = 0;

  if (showDnd) {
    cluster[count++] = dndIcon;
  }
  cluster[count++] = connectionIcon;
  cluster[count++] = battery.is_charging ? batteryChargeIcon : batteryIcon;

  int totalWidth = (count > 0) ? (count - 1) * ROUND_STATUS_ICON_GAP : 0;

  for (int i = 0; i < count; i++) {
    totalWidth += gbitmap_get_bounds(cluster[i]).size.w;
  }

  int x = (bounds.size.w - totalWidth) / 2;

  for (int i = 0; i < count; i++) {
    if (cluster[i] == batteryIcon || cluster[i] == batteryChargeIcon) {
      draw_battery(ctx, battery, x, ROUND_STATUS_CENTER_Y, color);
    } else {
      draw_icon(ctx, cluster[i], x, ROUND_STATUS_CENTER_Y);
    }
    x += gbitmap_get_bounds(cluster[i]).size.w + ROUND_STATUS_ICON_GAP;
  }
#else
  int centerY = bounds.size.h / 2;

  if (showDnd) {
    draw_icon(ctx, dndIcon, STATUS_BAR_PADDING_X, centerY);
  }

  int connW = gbitmap_get_bounds(connectionIcon).size.w;
  draw_icon(ctx, connectionIcon, (bounds.size.w - connW) / 2, centerY);

  int battW = gbitmap_get_bounds(batteryIcon).size.w;
  draw_battery(ctx, battery, bounds.size.w - STATUS_BAR_PADDING_X - battW,
               centerY, color);
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
  batteryIcon = gbitmap_create_with_resource(RESOURCE_ID_STATUS_BATTERY_BG);
  batteryChargeIcon =
      gbitmap_create_with_resource(RESOURCE_ID_STATUS_BATTERY_CHG);
  connectedIcon =
      gbitmap_create_with_resource(RESOURCE_ID_STATUS_WATCH_CONNECTED);
  disconnectedIcon =
      gbitmap_create_with_resource(RESOURCE_ID_STATUS_WATCH_DISCONNECTED);

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
  gbitmap_destroy(batteryIcon);
  gbitmap_destroy(batteryChargeIcon);
  gbitmap_destroy(connectedIcon);
  gbitmap_destroy(disconnectedIcon);
}

void StatusBar_redraw(void) {
#ifndef PBL_ROUND
  layer_set_frame(statusBarLayer, statusbar_frame());
#endif

  layer_set_hidden(statusBarLayer, !settings.enableStatusBar);
  layer_mark_dirty(statusBarLayer);
}
