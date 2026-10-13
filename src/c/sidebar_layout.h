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
 * stay stable). A preset says only which side the vertical sidebar goes
 * on: the bottom bar isn't a layout choice, it exists exactly when its
 * slots hold a widget. That keeps the two facts that used to be encoded
 * here -- "is there a bar" and "which style" -- derivable from state that
 * can't disagree with itself.
 */
typedef enum {
  LAYOUT_SIDEBAR_RIGHT = 0,
  LAYOUT_SIDEBAR_LEFT  = 1
} SidebarLayoutId;

static inline RegionEdge SidebarLayout_verticalEdge(uint8_t layoutId) {
  return (layoutId == LAYOUT_SIDEBAR_LEFT) ? REGION_EDGE_LEFT
                                           : REGION_EDGE_RIGHT;
}

#define MAX_REGION_SLOTS 6

// rendered slot caps: the vertical sidebar has room for a stack of
// small widgets, while the bar realistically fits two (three small on
// emery) -- the config page's fit budget polices the rest
#if defined(PBL_PLATFORM_EMERY)
#define VREGION_SLOT_CAP 6
#define HREGION_SLOT_CAP 3
#else
#define VREGION_SLOT_CAP 4
#define HREGION_SLOT_CAP 2
#endif
// the widest any platform's bar gets, so the slot array is one shape
#define HREGION_SLOT_MAX 3

// Region thicknesses. The wide sidebar is sized off the widest thing a
// widget routinely says -- a world time's "23:59" -- plus a pixel of
// margin on each side, so it is exactly as wide as it needs to be for a
// widget to show its full form. Measured with
// graphics_text_layout_get_content_size on the bold gothic the sidebar
// draws with: 36px at 18, 44px at 24, 49px at 28.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define VBAR_WIDTH_NARROW(largeFonts) ((largeFonts) ? 43 : 34)
#define VBAR_WIDTH_WIDE(largeFonts) ((largeFonts) ? 51 : 46)
#define HBAR_HEIGHT(largeFonts) ((largeFonts) ? 43 : 34)
#define REGION_V_PADDING 9
#else
#define VBAR_WIDTH_NARROW(largeFonts) 30
#define VBAR_WIDTH_WIDE(largeFonts) ((largeFonts) ? 46 : 38)
#define HBAR_HEIGHT(largeFonts) 30
#define REGION_V_PADDING 8
#endif

// the least space between two sidebar widgets that still reads as two
// widgets rather than one run-on stack; a sidebar too short to keep it
// (timeline quick view) drops widgets instead. The config page's fit
// budget uses the same figure (SIDEBAR_MIN_GAP in widgetMetrics.ts).
#define REGION_MIN_WIDGET_GAP 4

// The bar draws straight on the watchface, so the clock area may reach
// past the bar's top edge into the padding above its widgets: the
// clock's digits stop about a tenth of the area's height short of its
// lower bound (v_padding plus the font's baseline slop), so reserving
// the bar's full thickness shrinks the time far more than the bar
// actually costs. Reclaiming that slack keeps the digits near their
// no-bar size.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_CLOCK_ENCROACH 10
#else
#define HBAR_CLOCK_ENCROACH 0
#endif

// The horizontal bar centers its text within its own thickness, while
// the vertical bar's last slot is bottom-anchored (pinned
// REGION_V_PADDING above the screen edge). Those two anchor
// points don't naturally land on the same row, so the horizontal bar's
// content needs a small downward nudge to line up with it. Derived from
// ceil((HBAR_HEIGHT - textHeight) / 2) - REGION_V_PADDING for
// each platform/font-size combo.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_TEXT_NUDGE(largeFonts) ((largeFonts) ? 4 : 1)
#else
#define HBAR_TEXT_NUDGE(largeFonts) ((largeFonts) ? 0 : 2)
#endif

// horizontal bar gaps/padding
#define HBAR_VBAR_GAP 2
#define HBAR_SLOT_PADDING_X 4
#define HBAR_SLOT_PADDING_X_COMPACT 2

// bar widgets cluster around the bar's center rather than pinning to its
// ends, separated by this gap. Over-budget configs tighten the gap down
// to the compact value before the group crowds the end padding.
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define HBAR_WIDGET_GAP(largeFonts) 6
#else
#define HBAR_WIDGET_GAP(largeFonts) 4
#endif
#define HBAR_WIDGET_GAP_COMPACT 1

/*
 * Round watch arcs. Each arc is its own region holding a small stack of
 * widgets, centered on the arc's waist.
 *
 * An arc is a circle segment, so it is only at its full width across a
 * band around that waist -- towards the top and bottom of the screen it
 * tapers to nothing. That band, not the screen height, is what an arc's
 * widgets have to fit in: ROUND_ARC_CONTENT_BAND below. The config page
 * derives the same figure in arcContentBand() (utils/widgetMetrics.ts)
 * and budgets against it; the slot cap is only the coarse backstop.
 */
#if defined(PBL_PLATFORM_GABBRO)
#define ROUND_ARC_WIDTH 56
#define ROUND_ARC_CONTENT_WIDTH 42
#else
#define ROUND_ARC_WIDTH 40
#define ROUND_ARC_CONTENT_WIDTH 30
#endif
#define ROUND_ARC_SLOT_CAP 3

// The widget column is centered across the arc, nudged a pixel towards
// the screen center: the lens's midline drifts that way away from the
// waist, and measured off the emulator a centered stack's ink sat 1.5-2px
// towards the rim on both platforms. The offset is into the arc from its
// screen-center side, which is the layer's right edge on the left arc.
#define ROUND_ARC_INWARD_NUDGE 1
#define ROUND_ARC_CONTENT_INSET \
  ((ROUND_ARC_WIDTH - ROUND_ARC_CONTENT_WIDTH) / 2 - ROUND_ARC_INWARD_NUDGE)

/*
 * The band an arc's widgets stack inside.
 *
 * An arc is a lens between two curves, and away from its waist both of
 * them close in on the widget column: the arc's own edge (a circle of
 * radius `screen`) and the display's rim (radius `screen / 2`, so it
 * closes in about twice as fast and is the one that binds). The band is
 * how far the rim can come in before it reaches a widget.
 *
 * What it reaches is ink, not the content column: text is centered in
 * the column and rarely fills it, and icons are SIDEBAR_ICON_SIZE wide.
 * So the band is sized for an icon-wide column, leaving
 *
 *     s = (ROUND_ARC_WIDTH - SIDEBAR_ICON_SIZE) / 2 + ROUND_ARC_INWARD_NUDGE
 *
 * between the ink and the rim at the waist, and reaching to
 *
 *     d = sqrt(R^2 - (R - s)^2),  R = screen / 2
 *
 * either side of it. These are 2d, rounded down. Sizing for the full
 * content column instead cost gabbro over a third of its band (79 vs
 * 126px) and crushed three-widget stacks to 1px gaps. The config page
 * derives the same figures in arcContentBand().
 */
#if defined(PBL_PLATFORM_GABBRO)
#define ROUND_ARC_CONTENT_BAND 126
#else
#define ROUND_ARC_CONTENT_BAND 76
#endif

// Arc widgets are spread by this gap where the band has room for it,
// tightening towards the compact gap when it doesn't. On chalk that
// means anything past two small widgets; gabbro keeps most of its gap
// even for three. Better a crowded arc than one that spills off the lens.
#if defined(PBL_PLATFORM_GABBRO)
#define ROUND_ARC_WIDGET_GAP 12
#else
#define ROUND_ARC_WIDGET_GAP 8
#endif
#define ROUND_ARC_WIDGET_GAP_COMPACT 1
