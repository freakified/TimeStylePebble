#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"

void Sidebar_init(Window* window) {
  // load fonts
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // load the sidebar graphics
  #ifdef PBL_COLOR
    dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
    disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
    batteryImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
    batteryChargeImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  #else
    dateImage = gbitmap_create_with_resource(RESOURCE_ID_DATE_BG);
    disconnectImage = gbitmap_create_with_resource(RESOURCE_ID_DISCONNECTED);
    batteryImage = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_BG);
    batteryChargeImage = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
  #endif

  // init the sidebar layer
  if(!globalSettings.sidebarOnLeft) {
    sidebarLayer = layer_create(GRect(114, 0, 30, 168));
  } else {
    sidebarLayer = layer_create(GRect(0, 0, 30, 168));
  }

  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, updateSidebarLayer);
}

void Sidebar_deinit() {
  layer_destroy(sidebarLayer);

  #ifdef PBL_COLOR
    gdraw_command_image_destroy(dateImage);
    gdraw_command_image_destroy(disconnectImage);
    gdraw_command_image_destroy(batteryImage);
    gdraw_command_image_destroy(batteryChargeImage);
  #else
    gbitmap_destroy(dateImage);
    gbitmap_destroy(disconnectImage);
    gbitmap_destroy(batteryImage);
    gbitmap_destroy(batteryChargeImage);
  #endif
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
  // set all the date strings
  strftime(currentDayNum,  3, "%e", timeInfo);

  strncpy(currentDayName, dayNames[globalSettings.languageId][timeInfo->tm_wday], sizeof(currentDayName));
  strncpy(currentMonth, monthNames[globalSettings.languageId][timeInfo->tm_mon], sizeof(currentMonth));

  // remove padding on date num, if needed
  if(currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }

  // redraw the sidebar in case it changed in any way
  Sidebar_redraw();
}

void updateSidebarLayer(Layer *l, GContext* ctx) {
  if(globalSettings.useLargeFonts) {
    currentSidebarFont = lgSidebarFont;
    batteryFont = lgSidebarFont;
  } else {
    currentSidebarFont = mdSidebarFont;
    batteryFont = smSidebarFont;
  }


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

  if(!globalSettings.disableWeather) {
    if (Weather_currentWeatherIcon) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3, 7));
      #else
        graphics_draw_bitmap_in_rect(ctx, Weather_currentWeatherIcon, GRect(3, 7, 25, 25));
      #endif
    }

    // draw weather data only if it has been set
    if(Weather_weatherInfo.currentTemp != INT32_MIN) {

      int currentTemp = Weather_weatherInfo.currentTemp;

      if(!globalSettings.useMetric) {
        currentTemp = roundf(currentTemp * 1.8f + 32);
      }

      char tempString[8];

      // in large font mode, omit the degree symbol and move the text
      if(!globalSettings.useLargeFonts) {
        snprintf(tempString, sizeof(tempString), " %d°", currentTemp);

        graphics_draw_text(ctx,
                           tempString,
                           currentSidebarFont,
                           GRect(-5, 31, 38, 20),
                           GTextOverflowModeFill,
                           GTextAlignmentCenter,
                           NULL);
      } else {
        snprintf(tempString, sizeof(tempString), " %d", currentTemp);

        graphics_draw_text(ctx,
                           tempString,
                           currentSidebarFont,
                           GRect(-5, 27, 35, 20),
                           GTextOverflowModeFill,
                           GTextAlignmentCenter,
                           NULL);
      }


    }
  }

  // if the pebble is disconnected, display the disconnection image
  bool isPhoneConnected = bluetooth_connection_service_peek();

  if (!isPhoneConnected) {
    if(disconnectImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, disconnectImage, GPoint(3, 60));
      #else
        graphics_draw_bitmap_in_rect(ctx, disconnectImage, GRect(3, 60, 25, 25));
      #endif
    }
  }

  // only show the battery meter if the phone is connected (need room for the disconnection icon)
  // OR if the weather is disabled, in which case we have room
  if(isPhoneConnected || (!isPhoneConnected && globalSettings.disableWeather)) {
    if(globalSettings.showBatteryLevel) {
      if(!globalSettings.onlyShowBatteryWhenLow || (globalSettings.onlyShowBatteryWhenLow && battery_state_service_peek().charge_percent <= 20)) {
        drawBatteryStatus(ctx);
      }
    }
  }

  // in large font mode, draw a different date image
  if(!globalSettings.useLargeFonts) {
    if (dateImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, dateImage, GPoint(3, 118));
      #else
        graphics_draw_bitmap_in_rect(ctx, dateImage, GRect(3, 118, 25, 25));
      #endif
    }
  } else {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(2, 119, 26, 22), 2, GCornersAll);
    #else
      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorWhite);
      } else {
        graphics_context_set_fill_color(ctx, GColorBlack);
      }

      graphics_fill_rect(ctx, GRect(1, 119, 28, 22), 2, GCornersAll);

      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorBlack);
      } else {
        graphics_context_set_fill_color(ctx, GColorWhite);
      }

      graphics_fill_rect(ctx, GRect(3, 121, 24, 18), 0, GCornersAll);


    #endif

  }

  // color pebble should always use black for the date number...
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif

  int yPos = 0;

  yPos = globalSettings.useLargeFonts ? 113 : 121;

  graphics_draw_text(ctx,
                     currentDayNum,
                     currentSidebarFont,
                     GRect(0, yPos, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);


   // switch back to normal color for the rest
  #ifdef PBL_COLOR
    graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);
  #endif

  yPos = globalSettings.useLargeFonts ? 89 : 95;

  // now draw in the date info
  graphics_draw_text(ctx,
                     currentDayName,
                     currentSidebarFont,
                     GRect(-5, yPos, 40, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);




  // y position for month text
  yPos = globalSettings.useLargeFonts ? 137 : 142;

  graphics_draw_text(ctx,
                     currentMonth,
                     currentSidebarFont,
                     GRect(0, yPos, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

void drawBatteryStatus(GContext* ctx) {
  BatteryChargeState chargeState = battery_state_service_peek();
  char batteryString[6];

  int batteryPositionY = 63;

  // if the percentage indicator is enabled, ensure that the battery is still vertically centered
  if(globalSettings.showBatteryPct && !chargeState.is_charging) {
    batteryPositionY -= globalSettings.useLargeFonts ? 10 : 6;
  }



  // however, if the weather is disabled, put the battery where the weather was, at the top
  if(globalSettings.disableWeather) {
    batteryPositionY = 3;
  }

  if(chargeState.is_charging) {
    if(batteryChargeImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, batteryChargeImage, GPoint(3, batteryPositionY));
      #else
        graphics_draw_bitmap_in_rect(ctx, batteryChargeImage, GRect(3, batteryPositionY, 25, 25));
      #endif
    }
  } else {
    if (batteryImage) {
      #ifdef PBL_COLOR
        gdraw_command_image_draw(ctx, batteryImage, GPoint(3, batteryPositionY));
      #else
        graphics_draw_bitmap_in_rect(ctx, batteryImage, GRect(3, batteryPositionY, 25, 25));
      #endif
    }

    int width = roundf(18 * chargeState.charge_percent / 100.0f);

    graphics_context_set_fill_color(ctx, GColorBlack);

    #ifdef PBL_COLOR
      if(chargeState.charge_percent <= 20) {
        graphics_context_set_fill_color(ctx, GColorRed);
      }
    #else
      if(globalSettings.sidebarTextColor == GColorWhite) {
        graphics_context_set_fill_color(ctx, GColorWhite);
      }
    #endif

    graphics_fill_rect(ctx, GRect(6, 8 + batteryPositionY, width, 8), 0, GCornerNone);
  }

  // never show battery % while charging, because of this issue:
  // https://github.com/freakified/TimeStylePebble/issues/11
  if(globalSettings.showBatteryPct && !chargeState.is_charging) {

    if(!globalSettings.useLargeFonts) {
      snprintf(batteryString, sizeof(batteryString), "%d%%", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4, 18 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(batteryString, sizeof(batteryString), "%d", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4, 14 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }


  }
}
