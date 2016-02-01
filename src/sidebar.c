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
// layer update callbacks
void updateRectSidebar(Layer *l, GContext* ctx);

#ifdef PBL_ROUND
  void updateRoundSidebarLeft(Layer *l, GContext* ctx);
  void updateRoundSidebarRight(Layer *l, GContext* ctx);

  // shared drawing stuff between all layers
  void drawRoundSidebar(GContext* ctx, GRect bgBounds, SidebarWidgetType widgetType, int widgetXOffset);
#endif

Layer* sidebarLayer;

#ifdef PBL_ROUND
  Layer* sidebarLayer2;
#endif

void Sidebar_init(Window* window) {
  // init the sidebar layer
  GRect screenRect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds;

  #ifdef PBL_ROUND
    GRect bounds2;
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
    layer_set_update_proc(sidebarLayer, updateRoundSidebarLeft);
  #else
    layer_set_update_proc(sidebarLayer, updateRectSidebar);
  #endif

  #ifdef PBL_ROUND
    sidebarLayer2 = layer_create(bounds2);
    layer_add_child(window_get_root_layer(window), sidebarLayer2);
    layer_set_update_proc(sidebarLayer2, updateRoundSidebarRight);
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

  #ifdef PBL_ROUND
    layer_mark_dirty(sidebarLayer2);
  #endif
}

void Sidebar_updateTime(struct tm* timeInfo) {
  SidebarWidgets_updateTime(timeInfo);

  // redraw the sidebar in case it changed in any way
  Sidebar_redraw();
}

#ifdef PBL_ROUND

void updateRoundSidebarRight(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.h, bounds.size.h);

  drawRoundSidebar(ctx, bgBounds, globalSettings.widgets[2], 3);
}

void updateRoundSidebarLeft(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x - bounds.size.h + bounds.size.w, bounds.origin.y, bounds.size.h, bounds.size.h);

  bool isPhoneConnected = bluetooth_connection_service_peek();

  drawRoundSidebar(ctx, bgBounds, (isPhoneConnected) ? globalSettings.widgets[0] : BLUETOOTH_DISCONNECT, 7);
}

void drawRoundSidebar(GContext* ctx, GRect bgBounds, SidebarWidgetType widgetType, int widgetXOffset) {
  SidebarWidgets_updateFonts();

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  graphics_fill_radial(ctx,
                       bgBounds,
                       GOvalScaleModeFillCircle,
                       100,
                       DEG_TO_TRIGANGLE(0),
                       TRIG_MAX_ANGLE);

  SidebarWidgets_xOffset = widgetXOffset;
  SidebarWidget widget = getSidebarWidgetByType(widgetType);

  // calculate center position of the widget
  int widgetPosition = bgBounds.size.h / 2 - widget.getHeight() / 2;
  widget.draw(ctx, widgetPosition);
}
#endif

void updateRectSidebar(Layer *l, GContext* ctx) {
  SidebarWidgets_updateFonts();

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);

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
