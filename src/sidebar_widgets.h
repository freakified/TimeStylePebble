#pragma once
#include <pebble.h>

/*
 * "Compact Mode" is a global setting shared by all widgets, which determines
 * whether they should try to reduce their padding. Intended to allow larger
 * widgets to fit when vertical screen space is lacking
 */
extern bool SidebarWidgets_useCompactMode;

/*
 * The different types of sidebar widgets
 */
typedef enum {
  EMPTY,
  ALT_TIME_ZONE,
  DATE,
  BATTERY_METER,
  SECONDS,
  WEATHER_CURRENT,
  WEATHER_FORECAST_TODAY,
  WEATHER_FORECAST_TOMORROW,
  WEEK_NUMBER
} SidebarWidgetType;

typedef struct {
  /*
   * Returns the pixel height of the widget, taking into account all current
   * settings that would affect this, such as font size
   */
  int (*getHeight)();

  /*
   * Draws the widget using the provided graphics context
   */
  void (*draw)(GContext* ctx, int yPosition);
} SidebarWidget;

void SidebarWidgets_init();
void SidebarWidgets_deinit();
SidebarWidget getSidebarWidgetByType(SidebarWidgetType type);
void SidebarWidgets_updateFonts();
void SidebarWidgets_updateTime(struct tm* timeInfo);
