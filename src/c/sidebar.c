#include "sidebar.h"
#include "settings.h"
#include "sidebar_layout.h"
#include "sidebar_widgets.h"
#include <pebble.h>

/*
 * The sidebar is modeled as two "regions", each holding a list of widget
 * slots and a widget presentation mode; a single layout engine measures,
 * fits, and distributes the widgets along the region's main axis.
 *
 * On rect watches the regions are edges: a vertical sidebar (left/right)
 * and a horizontal bottom bar. On round the vertical region is the two
 * arcs, which split the vertical slot list between them -- slots 0..2 to
 * the left arc, 3..5 to the right -- and draw on top of the clock area
 * rather than reserving an edge; the bottom bar is a single widget
 * centered between the arcs' tapered ends.
 */

typedef struct {
  RegionEdge edge;                       // NONE = disabled
  SidebarLayoutMode widgetMode;          // how widgets present themselves
  bool dark;                             // "dark" appearance (bg-colored)
  int thickness;                         // width (vertical) / height (horizontal)
  int slotCount;                         // rendered cap already applied
  SidebarWidgetType slots[MAX_REGION_SLOTS];
} SidebarRegion;

static GRect screen_rect;

// while timeline quick view slides in or out, the bottom edge (in screen
// coordinates) the unobstructed area is heading for; INT16_MAX otherwise
static int16_t obstructionTargetBottom = INT16_MAX;
// rect: the vertical sidebar. round: the left arc.
static SidebarRegion vRegion;

static Layer *vRegionLayer;

#ifdef PBL_ROUND
// round: the right arc
static SidebarRegion vRegion2;
static Layer *vRegionLayer2;
#endif

static SidebarRegion hRegion;
static Layer *hRegionLayer;

static void sidebar_buildRegions(void) {
#ifdef PBL_ROUND
  // only the arc on the layout's side wears the sidebar color; the other
  // blends into the watchface, unless the user asks for both
  bool leftIsAccent =
      SidebarLayout_verticalEdge(settings.layoutId) == REGION_EDGE_LEFT;

  vRegion = (SidebarRegion){
      // arcs draw on top of the clock area, so they reserve no edge
      .edge = REGION_EDGE_NONE,
      .widgetMode = SIDEBAR_LAYOUT_STACKED,
      .dark = !settings.colorBothArcs && !leftIsAccent,
      .thickness = ROUND_ARC_WIDTH,
      .slotCount = ROUND_ARC_SLOT_CAP,
  };
  vRegion2 = (SidebarRegion){
      .edge = REGION_EDGE_NONE,
      .widgetMode = SIDEBAR_LAYOUT_STACKED,
      .dark = !settings.colorBothArcs && leftIsAccent,
      .thickness = ROUND_ARC_WIDTH,
      .slotCount = ROUND_ARC_SLOT_CAP,
  };

  // the vertical slot list is split down the middle: its first half is
  // the left arc's widget list, its second half the right arc's
  for (int i = 0; i < ROUND_ARC_SLOT_CAP; i++) {
    vRegion.slots[i] = (SidebarWidgetType)settings.vRegionSlots[i];
    vRegion2.slots[i] =
        (SidebarWidgetType)settings.vRegionSlots[ROUND_ARC_SLOT_CAP + i];
  }
#else
  vRegion = (SidebarRegion){
      .edge = SidebarLayout_verticalEdge(settings.layoutId),
      .widgetMode = SIDEBAR_LAYOUT_STACKED,
      // the sidebar always wears the accent background
      .dark = false,
      .thickness = settings.useWideSidebar
                       ? VBAR_WIDTH_WIDE(settings.useLargeFonts)
                       : VBAR_WIDTH_NARROW(settings.useLargeFonts),
      .slotCount = VREGION_SLOT_CAP,
  };

  for (int i = 0; i < VREGION_SLOT_CAP; i++) {
    vRegion.slots[i] = (SidebarWidgetType)settings.vRegionSlots[i];
  }
#endif

  // the bar exists exactly when it has something to show: an empty bar
  // would draw nothing while still costing the clock its height
  bool hasBarWidgets = false;
  for (int i = 0; i < HREGION_SLOT_CAP; i++) {
    if (settings.hRegionSlots[i] != EMPTY) {
      hasBarWidgets = true;
      break;
    }
  }

  hRegion = (SidebarRegion){
      .edge = hasBarWidgets ? REGION_EDGE_BOTTOM : REGION_EDGE_NONE,
      .widgetMode = SIDEBAR_LAYOUT_INLINE,
      // the bar blends into the watchface so the accent color stays with
      // the sidebar
      .dark = true,
#ifdef PBL_ROUND
      .thickness = ROUND_BAR_HEIGHT,
#else
      .thickness = HBAR_HEIGHT(settings.useLargeFonts),
#endif
      .slotCount = HREGION_SLOT_CAP,
  };

  for (int i = 0; i < hRegion.slotCount; i++) {
    hRegion.slots[i] = (SidebarWidgetType)settings.hRegionSlots[i];
  }
}

int Sidebar_reservedTop(void) {
  if (hRegion.edge == REGION_EDGE_TOP) {
    return hRegion.thickness;
  }
  return 0;
}

int Sidebar_reservedBottom(void) {
  if (hRegion.edge == REGION_EDGE_BOTTOM) {
    // a bar on the watchface lends the clock the padding above its widgets
#ifdef PBL_ROUND
    return hRegion.thickness + ROUND_BAR_BOTTOM_PADDING -
           ROUND_BAR_CLOCK_ENCROACH;
#else
    return hRegion.thickness - HBAR_CLOCK_ENCROACH;
#endif
  }
  return 0;
}

RegionEdge Sidebar_verticalEdge(void) { return vRegion.edge; }

int Sidebar_verticalThickness(void) {
  return vRegion.edge == REGION_EDGE_NONE ? 0 : vRegion.thickness;
}

#ifndef PBL_ROUND
static GRect vregion_frame(void) {
  int x = (vRegion.edge == REGION_EDGE_LEFT)
              ? 0
              : screen_rect.size.w - vRegion.thickness;
  return GRect(x, 0, vRegion.thickness, screen_rect.size.h);
}
#endif

// rect: the horizontal bar yields its corner to the vertical region.
// round: the bar is centered across the chord its widget sits on
static GRect hregion_frame(void) {
#ifdef PBL_ROUND
  return GRect((screen_rect.size.w - ROUND_BAR_WIDTH) / 2,
               screen_rect.size.h - ROUND_BAR_BOTTOM_PADDING -
                   hRegion.thickness,
               ROUND_BAR_WIDTH, hRegion.thickness);
#else
  int reserved = (vRegion.edge == REGION_EDGE_NONE)
                     ? 0
                     : vRegion.thickness + HBAR_VBAR_GAP;
  int x = (vRegion.edge == REGION_EDGE_LEFT) ? reserved : 0;
  int width = screen_rect.size.w - reserved;

  if (width < 0) {
    width = 0;
  }

  int y = (hRegion.edge == REGION_EDGE_TOP)
              ? 0
              : screen_rect.size.h - hRegion.thickness;
  return GRect(x, y, width, hRegion.thickness);
#endif
}

#ifndef PBL_ROUND

/*
 * Vertical flow: the slots form a packed widget list (EMPTY entries are
 * dropped): the first widget pins to the top, the last pins to the
 * bottom, and the rest spread with equal gaps between them. Widgets
 * always render their full form -- the config page's fit budget is what
 * keeps configs fitting at full height.
 *
 * When the sidebar is shorter than the stack needs (timeline quick view
 * covering the bottom of the screen), widgets are dropped rather than
 * crowded: interior widgets go first, last slot upwards, then the bottom
 * widget, so the top widget is the last to go.
 */
static void render_vertical_flow(SidebarRegion *r, Layer *l, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  SidebarMeasure sizes[MAX_REGION_SLOTS];
  int count = 0;

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, r->thickness);

  int totalHeight = 0;
  for (int i = 0; i < r->slotCount; i++) {
    if (r->slots[i] == EMPTY) {
      continue;
    }
    widgets[count] = getSidebarWidgetByType(r->slots[i]);
    sizes[count] = SidebarWidgets_measure(&widgets[count], &spec);
    totalHeight += sizes[count].h;
    count++;
  }

  // choose widgets for wherever quick view is headed, so they drop once
  // as it slides in rather than one by one as it covers them
  int available = bounds.size.h;
  int targetHeight = obstructionTargetBottom - bounds.origin.y;
  if (targetHeight < available) {
    available = targetHeight;
  }
  available -= REGION_V_PADDING * 2;

  while (count > 1 &&
         totalHeight + REGION_MIN_WIDGET_GAP * (count - 1) > available) {
    // the widget just above the bottom one, or the bottom one itself
    // once only the two pinned ends remain
    int drop = (count > 2) ? count - 2 : count - 1;
    totalHeight -= sizes[drop].h;
    for (int i = drop; i < count - 1; i++) {
      widgets[i] = widgets[i + 1];
      sizes[i] = sizes[i + 1];
    }
    count--;
  }

  if (count == 0) {
    return;
  }

  int topPos = REGION_V_PADDING;
  int lowerPos = bounds.size.h - REGION_V_PADDING - sizes[count - 1].h;

  GRect topFrame = GRect(0, topPos, r->thickness, sizes[0].h);
  SidebarWidgets_draw(ctx, &widgets[0], topFrame, &spec);

  if (count > 1) {
    GRect lowerFrame =
        GRect(0, lowerPos, r->thickness, sizes[count - 1].h);
    SidebarWidgets_draw(ctx, &widgets[count - 1], lowerFrame, &spec);
  }

  // interior widgets spread evenly between the pinned ends
  int interiorTotal = 0;
  int gapsLeft = 1;

  for (int i = 1; i < count - 1; i++) {
    interiorTotal += sizes[i].h;
    gapsLeft++;
  }

  int y = topPos + sizes[0].h;
  int remaining = (lowerPos - y) - interiorTotal;

  for (int i = 1; i < count - 1; i++) {
    int gap = remaining / gapsLeft;
    remaining -= gap;
    gapsLeft--;
    y += gap;
    GRect frame = GRect(0, y, r->thickness, sizes[i].h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
    y += sizes[i].h;
  }
}
#endif

/*
 * Horizontal flow: the slots form a packed widget list (EMPTY entries are
 * dropped) and the widgets cluster as one group in the middle of the bar,
 * separated by a fixed gap. Widgets always render their full form -- the
 * config page's fit budget is what keeps configs fitting. The only
 * over-budget concession is tightening the gap; past that, the group
 * eats into the end padding.
 */
static void render_horizontal_flow(SidebarRegion *r, Layer *l, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);
  int contentWidth = bounds.size.w - HBAR_SLOT_PADDING_X * 2;

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  SidebarMeasure sizes[MAX_REGION_SLOTS];
  int count = 0;

  for (int i = 0; i < r->slotCount; i++) {
    if (r->slots[i] == EMPTY) {
      continue;
    }
    widgets[count] = getSidebarWidgetByType(r->slots[i]);
    count++;
  }

  if (count == 0) {
    return;
  }

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, contentWidth);

  int totalWidth = 0;
  for (int i = 0; i < count; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
    totalWidth += sizes[i].w;
  }

  int gapCount = count - 1;
  int gap = HBAR_WIDGET_GAP(settings.useLargeFonts);

  // tighten the gap before letting the group spill past the end padding
  if (gapCount > 0 && totalWidth + gap * gapCount > contentWidth) {
    gap = (contentWidth - totalWidth) / gapCount;
    if (gap < HBAR_WIDGET_GAP_COMPACT) {
      gap = HBAR_WIDGET_GAP_COMPACT;
    }
  }

  int x = (bounds.size.w - (totalWidth + gap * gapCount)) / 2;
  if (x < HBAR_SLOT_PADDING_X_COMPACT) {
    x = HBAR_SLOT_PADDING_X_COMPACT;
  }

  int textNudge = HBAR_TEXT_NUDGE(settings.useLargeFonts);

  for (int i = 0; i < count; i++) {
    GRect frame = GRect(x, textNudge, sizes[i].w, bounds.size.h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
    x += sizes[i].w + gap;
  }
}

static void region_render(SidebarRegion *r, Layer *l, GContext *ctx) {
  if (r->edge == REGION_EDGE_NONE) {
    return;
  }

  const SidebarPalette *palette = r->dark
                                      ? &dynamicSettings.darkSidebarPalette
                                      : &dynamicSettings.sidebarPalette;
  SidebarWidgets_setActivePalette(palette);

  // round's bar skips its fill: it sits over the arcs' tapered ends,
  // and like a dark arc it would only paint the watchface over them
#ifndef PBL_ROUND
  graphics_context_set_fill_color(ctx, palette->background);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
#endif
  graphics_context_set_text_color(ctx, palette->text);

#ifdef PBL_ROUND
  render_horizontal_flow(r, l, ctx);
#else
  if (r->edge == REGION_EDGE_TOP || r->edge == REGION_EDGE_BOTTOM) {
    render_horizontal_flow(r, l, ctx);
  } else {
    render_vertical_flow(r, l, ctx);
  }
#endif
}

static void updateHRegionLayer(Layer *l, GContext *ctx) {
  region_render(&hRegion, l, ctx);
}

#ifndef PBL_ROUND
static void updateVRegionLayer(Layer *l, GContext *ctx) {
  region_render(&vRegion, l, ctx);
}
#else

/*
 * Arc flow: the slots form a packed widget list (EMPTY entries are
 * dropped) and the widgets stack as one block centered on the arc's
 * waist, the widest part of the lens. The column itself stays straight:
 * the lens narrows from both sides away from the waist, but its two
 * edges close in on each other almost symmetrically, so its midline
 * barely moves -- by the band's ends it has drifted 2px inwards on chalk
 * and under 4px on gabbro, with most of a stack's ink near the waist --
 * which is why the widgets are simply centered where the waist puts
 * them.
 *
 * Widgets always render their full form; the config page's arc budget is
 * what keeps configs fitting, and past it they simply crowd together.
 *
 * A "dark" arc skips its fill entirely rather than painting the
 * watchface color over itself: the arcs draw on top of the clock area,
 * so a filled seamless arc would punch a hole in the time.
 */
static void draw_round_arc(GContext *ctx, SidebarRegion *r, GRect bgBounds,
                           bool onLeft) {
  const SidebarPalette *palette = r->dark
                                      ? &dynamicSettings.darkSidebarPalette
                                      : &dynamicSettings.sidebarPalette;
  SidebarWidgets_setActivePalette(palette);

  if (!r->dark) {
    graphics_context_set_fill_color(ctx, palette->background);
    graphics_fill_radial(ctx, bgBounds, GOvalScaleModeFillCircle, 100,
                         DEG_TO_TRIGANGLE(0), TRIG_MAX_ANGLE);
  }
  graphics_context_set_text_color(ctx, palette->text);

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, ROUND_ARC_CONTENT_WIDTH);

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  SidebarMeasure sizes[MAX_REGION_SLOTS];
  int count = 0;
  int totalHeight = 0;

  for (int i = 0; i < r->slotCount; i++) {
    if (r->slots[i] == EMPTY) {
      continue;
    }
    widgets[count] = getSidebarWidgetByType(r->slots[i]);
    sizes[count] = SidebarWidgets_measure(&widgets[count], &spec);
    totalHeight += sizes[count].h;
    count++;
  }

  if (count == 0) {
    return;
  }

  // spread the stack out, but never past the band it has to fit in: a
  // crowded arc tightens its gaps before it spills off the lens
  int gapCount = count - 1;
  int gap = ROUND_ARC_WIDGET_GAP;

  if (gapCount > 0 && totalHeight + gap * gapCount > ROUND_ARC_CONTENT_BAND) {
    gap = (ROUND_ARC_CONTENT_BAND - totalHeight) / gapCount;
    if (gap < ROUND_ARC_WIDGET_GAP_COMPACT) {
      gap = ROUND_ARC_WIDGET_GAP_COMPACT;
    }
  }

  totalHeight += gap * gapCount;

  // the widget column sits ROUND_ARC_CONTENT_INSET in from the arc's
  // screen-center side: the layer's right edge on the left arc, its left
  // edge on the right one
  int x = onLeft ? ROUND_ARC_WIDTH - ROUND_ARC_CONTENT_WIDTH -
                       ROUND_ARC_CONTENT_INSET
                 : ROUND_ARC_CONTENT_INSET;

  // bgBounds is a square twice the arc's height, so a quarter of it is
  // the arc's waist
  int y = bgBounds.size.h / 4 - totalHeight / 2;

  for (int i = 0; i < count; i++) {
    GRect frame = GRect(x, y, ROUND_ARC_CONTENT_WIDTH, sizes[i].h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
    y += sizes[i].h + gap;
  }
}

static void updateRoundSidebarLeft(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds =
      GRect(bounds.origin.x - bounds.size.h * 2 + bounds.size.w,
            bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  draw_round_arc(ctx, &vRegion, bgBounds, true);
}

static void updateRoundSidebarRight(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.size.h / -2,
                         bounds.size.h * 2, bounds.size.h * 2);

  draw_round_arc(ctx, &vRegion2, bgBounds, false);
}

#endif

void Sidebar_init(Window *window) {
  Layer *root = window_get_root_layer(window);
  screen_rect = layer_get_bounds(root);

  sidebar_buildRegions();
  SidebarWidgets_init();

#ifdef PBL_ROUND
  vRegionLayer =
      layer_create(GRect(0, 0, ROUND_ARC_WIDTH, screen_rect.size.h));
  layer_set_update_proc(vRegionLayer, updateRoundSidebarLeft);
  layer_add_child(root, vRegionLayer);

  vRegionLayer2 =
      layer_create(GRect(screen_rect.size.w - ROUND_ARC_WIDTH, 0,
                         ROUND_ARC_WIDTH, screen_rect.size.h));
  layer_set_update_proc(vRegionLayer2, updateRoundSidebarRight);
  layer_add_child(root, vRegionLayer2);
#else
  vRegionLayer = layer_create(vregion_frame());
  layer_set_update_proc(vRegionLayer, updateVRegionLayer);
  layer_add_child(root, vRegionLayer);
#endif

  hRegionLayer = layer_create(hregion_frame());
  layer_set_update_proc(hRegionLayer, updateHRegionLayer);
  layer_add_child(root, hRegionLayer);
  layer_set_hidden(hRegionLayer, hRegion.edge == REGION_EDGE_NONE);
}

void Sidebar_deinit() {
  layer_destroy(vRegionLayer);

#ifdef PBL_ROUND
  layer_destroy(vRegionLayer2);
#endif
  layer_destroy(hRegionLayer);

  SidebarWidgets_deinit();
}

void Sidebar_redraw() {
  sidebar_buildRegions();

#ifndef PBL_ROUND
  layer_set_frame(vRegionLayer, vregion_frame());
#endif
  layer_set_frame(hRegionLayer, hregion_frame());
  layer_set_hidden(hRegionLayer, hRegion.edge == REGION_EDGE_NONE);

  layer_mark_dirty(vRegionLayer);

#ifdef PBL_ROUND
  layer_mark_dirty(vRegionLayer2);
#endif
  if (hRegion.edge != REGION_EDGE_NONE) {
    layer_mark_dirty(hRegionLayer);
  }
}

void Sidebar_obstructionWillChange(GRect finalUnobstructedArea) {
  obstructionTargetBottom =
      finalUnobstructedArea.origin.y + finalUnobstructedArea.size.h;
}

void Sidebar_obstructionDidChange(void) {
  obstructionTargetBottom = INT16_MAX;
  layer_mark_dirty(vRegionLayer);
}

void Sidebar_updateTime(struct tm *timeInfo) {
  SidebarWidgets_updateTime(timeInfo);
}
