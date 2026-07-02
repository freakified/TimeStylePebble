#pragma once
#include <pebble.h>

/*
 * The single home for sidebar region layout facts: which edges exist,
 * how many widget slots each region kind gets per platform, and how
 * thick each region is.
 */

// which screen edge a sidebar region occupies
typedef enum {
  REGION_EDGE_NONE   = 0,
  REGION_EDGE_RIGHT  = 1,
  REGION_EDGE_LEFT   = 2,
  REGION_EDGE_BOTTOM = 3,
  REGION_EDGE_TOP    = 4
} RegionEdge;

// presentation style for horizontal (top/bottom) bars
typedef enum {
  HBAR_STYLE_INLINE  = 0, // short bar, icon beside text
  HBAR_STYLE_STACKED = 1  // tall bar, icon above text (sidebar-style widgets)
} HBarStyle;

#define MAX_REGION_SLOTS 4

// rendered slot caps
#if defined(PBL_PLATFORM_EMERY)
#define VREGION_SLOT_CAP 4
#else
#define VREGION_SLOT_CAP 3
#endif
#define HREGION_SLOT_CAP_INLINE 2
#define HREGION_SLOT_CAP_STACKED 4

// region thicknesses
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define VBAR_WIDTH_NARROW(largeFonts) ((largeFonts) ? 43 : 34)
#define VBAR_WIDTH_WIDE(largeFonts) ((largeFonts) ? 58 : 52)
#define HBAR_HEIGHT_INLINE(largeFonts) ((largeFonts) ? 43 : 34)
#define HBAR_HEIGHT_STACKED(largeFonts) ((largeFonts) ? 62 : 56)
#define REGION_V_PADDING_DEFAULT 9
#else
#define VBAR_WIDTH_NARROW(largeFonts) 30
#define VBAR_WIDTH_WIDE(largeFonts) 46
#define HBAR_HEIGHT_INLINE(largeFonts) 30
#define HBAR_HEIGHT_STACKED(largeFonts) 50
#define REGION_V_PADDING_DEFAULT 8
#endif
#define REGION_V_PADDING_COMPACT 4

// horizontal bar gaps/padding
#define HBAR_VBAR_GAP 2
#define HBAR_SLOT_PADDING_X 4
#define HBAR_WIDGET_GAP 4

// round watch arcs
#if defined(PBL_PLATFORM_GABBRO)
#define ROUND_ARC_WIDTH 56
#define ROUND_ARC_CONTENT_WIDTH 42
#else
#define ROUND_ARC_WIDTH 40
#define ROUND_ARC_CONTENT_WIDTH 30
#endif
// widgets sit closer to the screen center than to the curved outer edge
#define ROUND_ARC_INNER_MARGIN 3
