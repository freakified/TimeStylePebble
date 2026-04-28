#pragma once
#include <pebble.h>

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define SIDEBAR_WIDTH 34
#else
#define SIDEBAR_WIDTH 30
#endif

// "public" functions
void Sidebar_init(Window* window);
void Sidebar_deinit();
void Sidebar_redraw();
void Sidebar_updateTime(struct tm* timeInfo);
