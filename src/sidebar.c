#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets/sidebar_widgets.h"

#define V_PADDING 8
#define SCREEN_HEIGHT 168

// "private" functions
void updateSidebarLayer(Layer *l, GContext* ctx);

void updateLeftSidebar(Layer *l, GContext* ctx);
void updateRightSidebar(Layer *l, GContext* ctx);

Layer* sidebarLayer;

#ifdef PBL_ROUND
  Layer* sidebarLayer2;
#endif

void Sidebar_init(Window* window) {
  // init the sidebar layer
  GRect screenRect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds, bounds2;

  #ifdef PBL_ROUND
    bounds = GRect(0, 0, 40, screenRect.size.h);
    bounds2 = GRect(screenRect.size.w - 40, 0, 40, screenRect.size.h);
  #else
    if(!globalSettings.sidebarOnLeft) {
      bounds = GRect(114, 0, 30, screenRect.size.h);
    } else {
      bounds = GRect(0, 0, 30, screenRect.size.h);
    }
  #endif

  // init the widgets
  SidebarWidgets_init();

  sidebarLayer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), sidebarLayer);

  #ifdef PBL_ROUND
    layer_set_update_proc(sidebarLayer, updateLeftSidebar);
  #else
    layer_set_update_proc(sidebarLayer, updateSidebarLayer);
  #endif

  #ifdef PBL_ROUND
    sidebarLayer2 = layer_create(bounds2);
    layer_add_child(window_get_root_layer(window), sidebarLayer2);
    layer_set_update_proc(sidebarLayer2, updateRightSidebar);
  #endif
}

void Sidebar_deinit() {
  layer_destroy(sidebarLayer);

  SidebarWidgets_deinit();
}

void Sidebar_redraw() {
  #ifndef PBL_ROUND
    // reposition the sidebar if needed
    if(globalSettings.sidebarOnLeft) {
      layer_set_frame(sidebarLayer, GRect(0, 0, 30, SCREEN_HEIGHT));
    } else {
      layer_set_frame(sidebarLayer, GRect(114, 0, 30, SCREEN_HEIGHT));
    }
  #endif

  // redraw the layer
  layer_mark_dirty(sidebarLayer);
}

void Sidebar_updateTime(struct tm* timeInfo) {
  SidebarWidgets_updateTime(timeInfo);

  // redraw the sidebar in case it changed in any way
  Sidebar_redraw();
}

void updateRightSidebar(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  GRect bounds = layer_get_bounds(l);

  #ifdef PBL_ROUND
    graphics_fill_radial(ctx,
                         GRect(bounds.origin.x, bounds.origin.y, bounds.size.h, bounds.size.h),
                         GOvalScaleModeFillCircle,
                         100,
                         DEG_TO_TRIGANGLE(0),
                         TRIG_MAX_ANGLE);
  #endif

  SidebarWidgets_xOffset = 3;
  updateSidebarLayer(l, ctx);
}

void updateLeftSidebar(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  GRect bounds = layer_get_bounds(l);

  #ifdef PBL_ROUND
    graphics_fill_radial(ctx,
                         GRect(bounds.origin.x - bounds.size.h + bounds.size.w, bounds.origin.y, bounds.size.h, bounds.size.h),
                         GOvalScaleModeFillCircle,
                         100,
                         DEG_TO_TRIGANGLE(0),
                         TRIG_MAX_ANGLE);
  #endif

  SidebarWidgets_xOffset = 7;
  updateSidebarLayer(l, ctx);
}

void updateSidebarLayer(Layer *l, GContext* ctx) {
  SidebarWidgets_updateFonts();

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  #ifndef PBL_ROUND
    graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  #endif

  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // on black and white pebbles, invert the icons if we're using the dark bar
  #ifndef PBL_COLOR
    if(globalSettings.sidebarTextColor == GColorWhite) {
      graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
    } else {
      graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    }
  #endif

  // if the pebble is connected, show the middle widget
  // otherwise, show the disconnection widget
  bool isPhoneConnected = bluetooth_connection_service_peek();
  // isPhoneConnected = false;

  SidebarWidget topWidget, middleWidget, lowerWidget;

  #ifdef PBL_ROUND
    topWidget = getSidebarWidgetByType(EMPTY);
    middleWidget = getSidebarWidgetByType(DATE);
    lowerWidget = getSidebarWidgetByType(EMPTY);
  #else
    topWidget = getSidebarWidgetByType(globalSettings.widgets[0]);
    middleWidget = getSidebarWidgetByType(globalSettings.widgets[1]);
    lowerWidget = getSidebarWidgetByType(globalSettings.widgets[2]);
  #endif


  // if the widgets are too tall, enable "compact mode"
  SidebarWidgets_useCompactMode = false; // ensure that we compare the non-compacted heights
  int totalHeight = topWidget.getHeight() + middleWidget.getHeight() + lowerWidget.getHeight();
  SidebarWidgets_useCompactMode = (totalHeight > 142) ? true : false;

  // if the phone is disconnected, replace the center widget with the BT disconnection indication
  if(!isPhoneConnected) {
    middleWidget = getSidebarWidgetByType(BLUETOOTH_DISCONNECT);
  }

  // calculate the three widget positions
  int topWidgetPos = V_PADDING;
  int lowerWidgetPos = SCREEN_HEIGHT - V_PADDING - lowerWidget.getHeight();

  // vertically center the middle widget using MATH
  int middleWidgetPos = ((lowerWidgetPos - middleWidget.getHeight()) + (topWidgetPos + topWidget.getHeight())) / 2;

  // draw the widgets
  topWidget.draw(ctx, topWidgetPos);
  middleWidget.draw(ctx, middleWidgetPos);
  lowerWidget.draw(ctx, lowerWidgetPos);

}
