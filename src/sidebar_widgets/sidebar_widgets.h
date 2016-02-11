#pragma once
#include <pebble.h>

/*
 * "Compact Mode" is a global setting shared by all widgets, which determines
 * whether they should try to reduce their padding. Intended to allow larger
 * widgets to fit when vertical screen space is lacking
 */
extern bool SidebarWidgets_useCompactMode;

/*
 * A global x offset used for nudging the widgets left and right
 * Included for round support
 */
extern int SidebarWidgets_xOffset;

/*
 * The different types of sidebar widgets:
 * we'll give them numbers so that we can index them in settings
 */
typedef enum {
  EMPTY                     = 0,
  BLUETOOTH_DISCONNECT      = 1,
  BATTERY_METER             = 2,
  ALT_TIME_ZONE             = 3,
  DATE                      = 4,
  SECONDS                   = 5,
  WEEK_NUMBER               = 6,
  WEATHER_CURRENT           = 7,
  WEATHER_FORECAST_TODAY    = 8,
  TIME                      = 9,
  HEALTH                    = 10
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
