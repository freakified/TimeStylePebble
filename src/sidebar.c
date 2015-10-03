#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets.h"

#define V_PADDING 8
#define SCREEN_HEIGHT 168

// "private" functions
void updateSidebarLayer(Layer *l, GContext* ctx);

Layer* sidebarLayer;

void Sidebar_init(Window* window) {
  // init the sidebar layer
  if(!globalSettings.sidebarOnLeft) {
    sidebarLayer = layer_create(GRect(114, 0, 30, SCREEN_HEIGHT));
  } else {
    sidebarLayer = layer_create(GRect(0, 0, 30, SCREEN_HEIGHT));
  }

  // init the widgets
  SidebarWidgets_init();

  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, updateSidebarLayer);
}

void Sidebar_deinit() {
  layer_destroy(sidebarLayer);

  SidebarWidgets_deinit();
}

void Sidebar_redraw() {
  // reposition the sidebar if needed
  if(globalSettings.sidebarOnLeft) {
    layer_set_frame(sidebarLayer, GRect(0, 0, 30, SCREEN_HEIGHT));
  } else {
    layer_set_frame(sidebarLayer, GRect(114, 0, 30, SCREEN_HEIGHT));
  }

  // redraw the layer
  layer_mark_dirty(sidebarLayer);
}

void Sidebar_updateTime(struct tm* timeInfo) {
  SidebarWidgets_updateTime(timeInfo);

  // redraw the sidebar in case it changed in any way
  Sidebar_redraw();
}

void updateSidebarLayer(Layer *l, GContext* ctx) {
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

  SidebarWidget topWidget = getSidebarWidgetByType(globalSettings.widgets[0]);
  SidebarWidget middleWidget = getSidebarWidgetByType(globalSettings.widgets[1]);
  SidebarWidget lowerWidget = getSidebarWidgetByType(globalSettings.widgets[2]);

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
