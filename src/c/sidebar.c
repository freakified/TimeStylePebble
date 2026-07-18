#include "sidebar.h"
#include "settings.h"
#include "sidebar_layout.h"
#include "sidebar_widgets.h"
#include <pebble.h>

/*
 * The sidebar is modeled as up to two edge "regions": a vertical one
 * (left/right) and a horizontal one (top/bottom). Each region holds a list
 * of widget slots and a widget presentation mode; a single layout engine
 * measures, fits, and distributes the widgets along the region's main axis.
 * Round watches present the vertical region as two arc layers instead.
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
static SidebarRegion vRegion;

static Layer *vRegionLayer;

#ifdef PBL_ROUND
static Layer *vRegionLayer2;
#else
static SidebarRegion hRegion;
static Layer *hRegionLayer;
#endif

static void sidebar_buildRegions(void) {
#ifdef PBL_ROUND
  vRegion = (SidebarRegion){
      // arcs draw on top of the clock area, so they reserve no edge
      .edge = REGION_EDGE_NONE,
      .widgetMode = SIDEBAR_LAYOUT_STACKED,
      .thickness = ROUND_ARC_WIDTH,
      .slotCount = 2,
  };

  // slots are a packed list (EMPTY is only wire padding): the first
  // widget takes the left arc, the second the right
  for (int i = 0, arc = 0; i < MAX_REGION_SLOTS && arc < 2; i++) {
    if (settings.vRegionSlots[i] != EMPTY) {
      vRegion.slots[arc] = (SidebarWidgetType)settings.vRegionSlots[i];
      arc++;
    }
  }
#else
  RegionEdge vEdge = SidebarLayout_verticalEdge(settings.layoutId);

  vRegion = (SidebarRegion){
      .edge = vEdge,
      .widgetMode = SIDEBAR_LAYOUT_STACKED,
      // seamless regions blend into the watchface, and the extra room
      // lets the vertical sidebar widen
      .dark = settings.useSeamlessSidebar,
      .thickness = (vEdge == REGION_EDGE_NONE) ? 0
                   : settings.useSeamlessSidebar
                       ? VBAR_WIDTH_WIDE(settings.useLargeFonts)
                       : VBAR_WIDTH_NARROW(settings.useLargeFonts),
      .slotCount = VREGION_SLOT_CAP,
  };

  for (int i = 0; i < VREGION_SLOT_CAP; i++) {
    vRegion.slots[i] = (SidebarWidgetType)settings.vRegionSlots[i];
  }

  bool stacked = (SidebarLayout_hBarStyle(settings.layoutId) == HBAR_STYLE_STACKED);
  RegionEdge hEdge = SidebarLayout_horizontalEdge(settings.layoutId);

  hRegion = (SidebarRegion){
      .edge = hEdge,
      .widgetMode = stacked ? SIDEBAR_LAYOUT_STACKED : SIDEBAR_LAYOUT_INLINE,
      // a secondary bar always blends into the watchface; when the bar
      // is the only region it follows the seamless setting
      .dark = (vEdge != REGION_EDGE_NONE) || settings.useSeamlessSidebar,
      .thickness = stacked ? HBAR_HEIGHT_STACKED(settings.useLargeFonts)
                           : HBAR_HEIGHT_INLINE(settings.useLargeFonts),
      .slotCount = stacked ? HREGION_SLOT_CAP_STACKED : HREGION_SLOT_CAP_INLINE,
  };

  for (int i = 0; i < hRegion.slotCount; i++) {
    hRegion.slots[i] = (SidebarWidgetType)settings.hRegionSlots[i];
  }
#endif
}

int Sidebar_reservedTop(void) {
#ifndef PBL_ROUND
  if (hRegion.edge == REGION_EDGE_TOP) {
    return hRegion.thickness;
  }
#endif
  return 0;
}

int Sidebar_reservedBottom(void) {
#ifndef PBL_ROUND
  if (hRegion.edge == REGION_EDGE_BOTTOM) {
    return hRegion.thickness;
  }
#endif
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

// the horizontal bar yields its corner to the vertical region
static GRect hregion_frame(void) {
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
}

/*
 * Vertical flow: the slots form a packed widget list (EMPTY entries are
 * dropped): the first widget pins to the top, the last pins to the
 * bottom, and the rest spread with equal gaps between them. Widgets
 * always render their full form -- the config page's unit budget is what
 * keeps configs fitting. The only over-budget concession is shrinking
 * the end padding; past that, widgets simply crowd together.
 */
static void render_vertical_flow(SidebarRegion *r, Layer *l, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);

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
      SidebarWidgets_makeRenderSpec(r->widgetMode, r->thickness, false);

  int totalHeight = 0;
  for (int i = 0; i < count; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
    totalHeight += sizes[i].h;
  }

  int v_padding =
      (totalHeight > bounds.size.h - REGION_V_PADDING_DEFAULT * 2)
          ? REGION_V_PADDING_COMPACT
          : REGION_V_PADDING_DEFAULT;

  int topPos = v_padding;
  int lowerPos = bounds.size.h - v_padding - sizes[count - 1].h;

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

/*
 * Horizontal flow: EMPTY slots are dropped, the rest render at their
 * natural width and distribute space-evenly across the bar. Widgets
 * always get their full form when the config fits its unit budget; only
 * over-budget configs degrade, deterministically: the widest widgets
 * shrink toward an equal share (narrower ones keep their natural width)
 * and their text candidates absorb the squeeze.
 */
static void render_horizontal_flow(SidebarRegion *r, Layer *l, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);
  int contentWidth = bounds.size.w - HBAR_SLOT_PADDING_X * 2;

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  SidebarMeasure sizes[MAX_REGION_SLOTS];
  int visibleCount = 0;

  for (int i = 0; i < r->slotCount; i++) {
    if (r->slots[i] == EMPTY) {
      continue;
    }
    widgets[visibleCount] = getSidebarWidgetByType(r->slots[i]);
    visibleCount++;
  }

  if (visibleCount == 0) {
    return;
  }

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, contentWidth, true);

  int totalWidth = 0;
  for (int i = 0; i < visibleCount; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
    totalWidth += sizes[i].w;
  }

  int available = contentWidth - HBAR_WIDGET_GAP * (visibleCount - 1);

  if (available < 0) {
    available = 0;
  }

  if (totalWidth > available) {
    // find the equal share by fixing widgets that fit under it: each
    // pass locks in widgets narrower than the current share and
    // redistributes what they didn't use; the rest split what's left
    bool fits[MAX_REGION_SLOTS] = {false};
    int budget = available;
    int flexCount = visibleCount;
    bool changed = true;

    while (changed && flexCount > 0) {
      changed = false;
      int share = budget / flexCount;
      for (int i = 0; i < visibleCount; i++) {
        if (!fits[i] && sizes[i].w <= share) {
          fits[i] = true;
          budget -= sizes[i].w;
          flexCount--;
          changed = true;
        }
      }
    }

    totalWidth = 0;
    for (int i = 0; i < visibleCount; i++) {
      if (!fits[i]) {
        int share = budget / flexCount;
        sizes[i].w = share;
        budget -= share;
        flexCount--;
      }
      totalWidth += sizes[i].w;
    }
  }

  int remaining = bounds.size.w - totalWidth;
  int gapsLeft = visibleCount + 1;
  int x = 0;
  int textNudge = HBAR_INLINE_TEXT_NUDGE(settings.useLargeFonts);

  for (int i = 0; i < visibleCount; i++) {
    int gap = remaining / gapsLeft;
    remaining -= gap;
    gapsLeft--;
    x += gap;
    GRect frame = GRect(x, textNudge, sizes[i].w, bounds.size.h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
    x += sizes[i].w;
  }
}

/*
 * Stacked bar flow: EMPTY slots are dropped, the bar is split into equal
 * columns, and each widget renders in the stacked (sidebar-style) layout
 * centered within its column.
 */
static void render_stacked_bar_flow(SidebarRegion *r, Layer *l,
                                    GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  int visibleCount = 0;

  for (int i = 0; i < r->slotCount; i++) {
    if (r->slots[i] == EMPTY) {
      continue;
    }
    widgets[visibleCount] = getSidebarWidgetByType(r->slots[i]);
    visibleCount++;
  }

  if (visibleCount == 0) {
    return;
  }

  int usableWidth = bounds.size.w - HBAR_SLOT_PADDING_X * 2;
  int columnWidth = usableWidth / visibleCount;

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, columnWidth, true);

  SidebarMeasure sizes[MAX_REGION_SLOTS];

  for (int i = 0; i < visibleCount; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
  }

  for (int i = 0; i < visibleCount; i++) {
    int x = HBAR_SLOT_PADDING_X + columnWidth * i;
    int y = (bounds.size.h - sizes[i].h) / 2;
    GRect frame = GRect(x, y, columnWidth, sizes[i].h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
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

  graphics_context_set_fill_color(ctx, palette->background);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  graphics_context_set_text_color(ctx, palette->text);

  if (r->edge == REGION_EDGE_TOP || r->edge == REGION_EDGE_BOTTOM) {
    if (r->widgetMode == SIDEBAR_LAYOUT_STACKED) {
      render_stacked_bar_flow(r, l, ctx);
    } else {
      render_horizontal_flow(r, l, ctx);
    }
  } else {
    render_vertical_flow(r, l, ctx);
  }
}

static void updateVRegionLayer(Layer *l, GContext *ctx) {
  region_render(&vRegion, l, ctx);
}

static void updateHRegionLayer(Layer *l, GContext *ctx) {
  region_render(&hRegion, l, ctx);
}

#else /* PBL_ROUND */

static void draw_round_arc(GContext *ctx, GRect bgBounds,
                           SidebarWidgetType widgetType, int widgetXOffset) {
  SidebarWidgets_setActivePalette(&dynamicSettings.sidebarPalette);
  graphics_context_set_fill_color(ctx, settings.sidebarColor);
  graphics_fill_radial(ctx, bgBounds, GOvalScaleModeFillCircle, 100,
                       DEG_TO_TRIGANGLE(0), TRIG_MAX_ANGLE);

  SidebarRenderSpec spec = SidebarWidgets_makeRenderSpec(
      vRegion.widgetMode, ROUND_ARC_CONTENT_WIDTH, false);
  SidebarWidgetDefinition widget = getSidebarWidgetByType(widgetType);
  SidebarMeasure widgetSize = SidebarWidgets_measure(&widget, &spec);

  // center the widget vertically within the arc
  int widgetPosition = bgBounds.size.h / 4 - widgetSize.h / 2;
  GRect widgetFrame = GRect(widgetXOffset, widgetPosition,
                            ROUND_ARC_CONTENT_WIDTH, widgetSize.h);
  SidebarWidgets_draw(ctx, &widget, widgetFrame, &spec);
}

static void updateRoundSidebarLeft(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds =
      GRect(bounds.origin.x - bounds.size.h * 2 + bounds.size.w,
            bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  draw_round_arc(ctx, bgBounds, vRegion.slots[0],
                 ROUND_ARC_WIDTH - ROUND_ARC_CONTENT_WIDTH -
                     ROUND_ARC_INNER_MARGIN);
}

static void updateRoundSidebarRight(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.size.h / -2,
                         bounds.size.h * 2, bounds.size.h * 2);

  draw_round_arc(ctx, bgBounds, vRegion.slots[1], ROUND_ARC_INNER_MARGIN);
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

  hRegionLayer = layer_create(hregion_frame());
  layer_set_update_proc(hRegionLayer, updateHRegionLayer);
  layer_add_child(root, hRegionLayer);
  layer_set_hidden(hRegionLayer, hRegion.edge == REGION_EDGE_NONE);
#endif
}

void Sidebar_deinit() {
  layer_destroy(vRegionLayer);

#ifdef PBL_ROUND
  layer_destroy(vRegionLayer2);
#else
  layer_destroy(hRegionLayer);
#endif

  SidebarWidgets_deinit();
}

void Sidebar_redraw() {
  sidebar_buildRegions();

#ifndef PBL_ROUND
  layer_set_frame(vRegionLayer, vregion_frame());
  layer_set_frame(hRegionLayer, hregion_frame());
  layer_set_hidden(hRegionLayer, hRegion.edge == REGION_EDGE_NONE);
#endif

  layer_mark_dirty(vRegionLayer);

#ifdef PBL_ROUND
  layer_mark_dirty(vRegionLayer2);
#else
  if (hRegion.edge != REGION_EDGE_NONE) {
    layer_mark_dirty(hRegionLayer);
  }
#endif
}

void Sidebar_updateTime(struct tm *timeInfo) {
  SidebarWidgets_updateTime(timeInfo);
}
