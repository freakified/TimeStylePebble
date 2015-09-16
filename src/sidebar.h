#pragma once
#include <pebble.h>

Layer* sidebarLayer;

// sidebar icons
#ifdef PBL_COLOR
  GDrawCommandImage* dateImage;
  GDrawCommandImage* disconnectImage;
  GDrawCommandImage* batteryImage;
  GDrawCommandImage* batteryChargeImage;
#else
  GBitmap* dateImage;
  GBitmap* disconnectImage;
  GBitmap* batteryImage;
  GBitmap* batteryChargeImage;
#endif

// fonts
GFont smSidebarFont;
GFont mdSidebarFont;
GFont lgSidebarFont;
GFont currentSidebarFont;
GFont batteryFont;

// the date and weather strings
char currentDayName[8];
char currentDayNum[8];
char currentMonth[8];

// "public" functions
void Sidebar_init(Window* window);
void Sidebar_deinit();
void Sidebar_redraw();
void Sidebar_updateTime(struct tm* timeInfo);

// "private" functions
void drawBatteryStatus(GContext* ctx);
void updateSidebarLayer(Layer *l, GContext* ctx);
