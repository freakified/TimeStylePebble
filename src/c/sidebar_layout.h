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

/*
 * User-facing layout presets (the ids travel over AppMessage, so they must
 * stay stable). A preset says only where the vertical sidebar goes: the
 * bottom bar isn't a layout choice, it exists exactly when its slots hold
 * a widget, and it presents itself inline beside a sidebar or stacked when
 * it's the only region. That keeps the two facts that used to be encoded
 * here -- "is there a bar" and "which style" -- derivable from state that
 * can't disagree with itself.
 */
typedef enum {
  LAYOUT_SIDEBAR_RIGHT = 0,
  LAYOUT_SIDEBAR_LEFT  = 1,
  LAYOUT_NO_SIDEBAR    = 2
} SidebarLayoutId;

static inline RegionEdge SidebarLayout_verticalEdge(uint8_t layoutId) {
  switch (layoutId) {
    case LAYOUT_SIDEBAR_LEFT:
      return REGION_EDGE_LEFT;
    case LAYOUT_NO_SIDEBAR:
      return REGION_EDGE_NONE;
    default:
      return REGION_EDGE_RIGHT;
  }
}

#define MAX_REGION_SLOTS 6

// rendered slot caps: the vertical sidebar has room for a stack of
// small widgets, while the inline bar realistically fits two (three
// small on emery) -- the config page's unit budget polices the rest
#if defined(PBL_PLATFORM_EMERY)
#define VREGION_SLOT_CAP 6
#define HREGION_SLOT_CAP_INLINE 3
#else
#define VREGION_SLOT_CAP 4
#define HREGION_SLOT_CAP_INLINE 2
#endif
#define HREGION_SLOT_CAP_STACKED 4

// Region thicknesses. The wide sidebar is sized off the widest thing a
// widget routinely says -- a world time's "23:59" -- plus a pixel of
// margin on each side, so it is exactly as wide as it needs to be for a
// widget to show its full form. Measured with
// graphics_text_layout_get_content_size on the bold gothic the sidebar
// draws with: 36px at 18, 44px at 24, 49px at 28.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define VBAR_WIDTH_NARROW(largeFonts) ((largeFonts) ? 43 : 34)
#define VBAR_WIDTH_WIDE(largeFonts) ((largeFonts) ? 51 : 46)
#define HBAR_HEIGHT_INLINE(largeFonts) ((largeFonts) ? 43 : 34)
#define HBAR_HEIGHT_STACKED(largeFonts) ((largeFonts) ? 62 : 56)
#define REGION_V_PADDING_DEFAULT 9
#else
#define VBAR_WIDTH_NARROW(largeFonts) 30
#define VBAR_WIDTH_WIDE(largeFonts) ((largeFonts) ? 46 : 38)
#define HBAR_HEIGHT_INLINE(largeFonts) 30
#define HBAR_HEIGHT_STACKED(largeFonts) 50
#define REGION_V_PADDING_DEFAULT 8
#endif
#define REGION_V_PADDING_COMPACT 4

// An inline bar beside a sidebar draws straight on the watchface, so the
// clock area may reach past the bar's top edge into the padding above its
// widgets: the clock's digits stop about a tenth of the area's height
// short of its lower bound (v_padding plus the font's baseline slop), so
// reserving the bar's full thickness shrinks the time far more than the
// bar actually costs. Reclaiming that slack keeps the digits near their
// no-bar size.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_CLOCK_ENCROACH 10
#else
#define HBAR_CLOCK_ENCROACH 0
#endif

// The inline horizontal bar centers its text within its own thickness,
// while the vertical bar's last slot is bottom-anchored (pinned
// REGION_V_PADDING_DEFAULT above the screen edge). Those two anchor
// points don't naturally land on the same row, so the horizontal bar's
// content needs a small downward nudge to line up with it. Derived from
// ceil((HBAR_HEIGHT_INLINE - textHeight) / 2) - REGION_V_PADDING_DEFAULT
// for each platform/font-size combo.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_INLINE_TEXT_NUDGE(largeFonts) ((largeFonts) ? 4 : 1)
#else
#define HBAR_INLINE_TEXT_NUDGE(largeFonts) ((largeFonts) ? 0 : 2)
#endif

// horizontal bar gaps/padding
#define HBAR_VBAR_GAP 2
#define HBAR_SLOT_PADDING_X 4
#define HBAR_SLOT_PADDING_X_COMPACT 2

// inline bar widgets cluster around the bar's center rather than pinning
// to its ends, separated by this gap. Over-budget configs tighten the gap
// down to the compact value before the group crowds the end padding.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_WIDGET_GAP(largeFonts) 6
#else
#define HBAR_WIDGET_GAP(largeFonts) 4
#endif
#define HBAR_WIDGET_GAP_COMPACT 1

// a stacked bar widget is a vertical sidebar widget wearing a different
// hat, so its column never needs to be wider than the widest sidebar.
// Capping it keeps a sparse stacked bar clustered in the middle instead
// of marooning two widgets at the bar's quarter points.
#define HBAR_STACKED_COLUMN_MAX(largeFonts) VBAR_WIDTH_WIDE(largeFonts)

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
