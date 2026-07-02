#pragma once
#include <pebble.h>
#include "sidebar_layout.h"

// "public" functions
void Sidebar_init(Window* window);
void Sidebar_deinit();
void Sidebar_redraw();
void Sidebar_updateTime(struct tm* timeInfo);

// screen-edge reservations, for the clock area to compute its frame
int Sidebar_reservedTop(void);
int Sidebar_reservedBottom(void);
RegionEdge Sidebar_verticalEdge(void);
int Sidebar_verticalThickness(void);
