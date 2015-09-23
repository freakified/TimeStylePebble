#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets.h"

// "private" functions
void updateSidebarLayer(Layer *l, GContext* ctx);

Layer* sidebarLayer;

void Sidebar_init(Window* window) {
  // init the sidebar layer
  if(!globalSettings.sidebarOnLeft) {
    sidebarLayer = layer_create(GRect(114, 0, 30, 168));
  } else {
    sidebarLayer = layer_create(GRect(0, 0, 30, 168));
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
    layer_set_frame(sidebarLayer, GRect(0, 0, 30, 168));
  } else {
    layer_set_frame(sidebarLayer, GRect(114, 0, 30, 168));
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

  getSidebarWidgetByType(BATTERY_METER).draw(ctx, 5);

  getSidebarWidgetByType(DATE).draw(ctx, 95);

  getSidebarWidgetByType(EMPTY).draw(ctx, 50);

  //
  // if(!globalSettings.disableWeather) {
  //   if (Weather_currentWeatherIcon) {
  //     #ifdef PBL_COLOR
  //       gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3, 7));
  //     #else
  //       graphics_draw_bitmap_in_rect(ctx, Weather_currentWeatherIcon, GRect(3, 7, 25, 25));
  //     #endif
  //   }
  //
  //   // draw weather data only if it has been set
  //   if(Weather_weatherInfo.currentTemp != INT32_MIN) {
  //
  //     int currentTemp = Weather_weatherInfo.currentTemp;
  //
  //     if(!globalSettings.useMetric) {
  //       currentTemp = roundf(currentTemp * 1.8f + 32);
  //     }
  //
  //     char tempString[8];
  //
  //     // in large font mode, omit the degree symbol and move the text
  //     if(!globalSettings.useLargeFonts) {
  //       snprintf(tempString, sizeof(tempString), " %d°", currentTemp);
  //
  //       graphics_draw_text(ctx,
  //                          tempString,
  //                          currentSidebarFont,
  //                          GRect(-5, 31, 38, 20),
  //                          GTextOverflowModeFill,
  //                          GTextAlignmentCenter,
  //                          NULL);
  //     } else {
  //       snprintf(tempString, sizeof(tempString), " %d", currentTemp);
  //
  //       graphics_draw_text(ctx,
  //                          tempString,
  //                          currentSidebarFont,
  //                          GRect(-5, 27, 35, 20),
  //                          GTextOverflowModeFill,
  //                          GTextAlignmentCenter,
  //                          NULL);
  //     }
  //
  //
  //   }
  // }
  //
  // // if the pebble is disconnected, display the disconnection image
  // bool isPhoneConnected = bluetooth_connection_service_peek();
  //
  // if (!isPhoneConnected) {
  //   if(disconnectImage) {
  //     #ifdef PBL_COLOR
  //       gdraw_command_image_draw(ctx, disconnectImage, GPoint(3, 60));
  //     #else
  //       graphics_draw_bitmap_in_rect(ctx, disconnectImage, GRect(3, 60, 25, 25));
  //     #endif
  //   }
  // }
  //
  //
}
