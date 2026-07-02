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
      .widgetMode = SIDEBAR_LAYOUT_VERTICAL_NARROW,
      .thickness = ROUND_ARC_WIDTH,
      .slotCount = 2,
      .slots = {settings.widgets[0], settings.widgets[2], EMPTY, EMPTY},
  };
#else
  vRegion = (SidebarRegion){
      .edge = settings.sidebarOnLeft ? REGION_EDGE_LEFT : REGION_EDGE_RIGHT,
      .widgetMode = SIDEBAR_LAYOUT_VERTICAL_NARROW,
      .thickness = settings.useWideSidebar
                       ? VBAR_WIDTH_WIDE(settings.useLargeFonts)
                       : VBAR_WIDTH_NARROW(settings.useLargeFonts),
      .slotCount = 3,
      .slots = {settings.widgets[0], settings.widgets[1], settings.widgets[2],
                EMPTY},
  };
  hRegion = (SidebarRegion){
      .edge = settings.useBottomBar ? REGION_EDGE_BOTTOM : REGION_EDGE_NONE,
      .widgetMode = SIDEBAR_LAYOUT_HORIZONTAL,
      .thickness = HBAR_HEIGHT_INLINE(settings.useLargeFonts),
      .slotCount = 2,
      .slots = {settings.bottomBarWidgets[0], settings.bottomBarWidgets[1],
                EMPTY, EMPTY},
  };
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
  int x = (vRegion.edge == REGION_EDGE_LEFT)
              ? vRegion.thickness + HBAR_VBAR_GAP
              : 0;
  int width = screen_rect.size.w - vRegion.thickness - HBAR_VBAR_GAP;

  if (width < 0) {
    width = 0;
  }

  int y = (hRegion.edge == REGION_EDGE_TOP)
              ? 0
              : screen_rect.size.h - hRegion.thickness;
  return GRect(x, y, width, hRegion.thickness);
}

/*
 * Vertical flow: first and last slots pin to the region's ends, interior
 * slots spread with equal gaps between them. If the widgets don't fit,
 * fall back in stages: compact widget text, then hide interior widgets
 * from the middle out, then shrink the end padding.
 */
static void render_vertical_flow(SidebarRegion *r, Layer *l, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(l);
  int count = r->slotCount;

  SidebarWidgetDefinition widgets[MAX_REGION_SLOTS];
  SidebarMeasure sizes[MAX_REGION_SLOTS];
  bool hidden[MAX_REGION_SLOTS] = {false};

  for (int i = 0; i < count; i++) {
    widgets[i] = getSidebarWidgetByType(r->slots[i]);
  }

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, r->thickness, false);

  int fit_threshold = bounds.size.h - REGION_V_PADDING_DEFAULT * 2;
  int v_padding = REGION_V_PADDING_DEFAULT;

  int totalHeight = 0;
  for (int i = 0; i < count; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
    totalHeight += sizes[i].h;
  }

  // fallback: compact the widget text
  if (totalHeight > fit_threshold) {
    spec.compact = true;
    totalHeight = 0;
    for (int i = 0; i < count; i++) {
      sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
      totalHeight += sizes[i].h;
    }
  }

  // fallback: hide interior widgets, middle-out
  for (int step = 0; totalHeight > fit_threshold && step < count - 2; step++) {
    int mid = count / 2 + ((step % 2 == 0) ? -(step / 2) : (step / 2 + 1));
    if (count % 2 == 0) {
      mid -= (step % 2 == 0) ? 1 : 0;
    }
    if (mid <= 0 || mid >= count - 1 || hidden[mid]) {
      continue;
    }
    hidden[mid] = true;
    totalHeight -= sizes[mid].h;
  }

  // fallback: shrink the end padding
  if (totalHeight > fit_threshold) {
    v_padding = REGION_V_PADDING_COMPACT;
  }

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
    if (!hidden[i]) {
      interiorTotal += sizes[i].h;
      gapsLeft++;
    }
  }

  int y = topPos + sizes[0].h;
  int remaining = (lowerPos - y) - interiorTotal;

  for (int i = 1; i < count - 1; i++) {
    if (hidden[i]) {
      continue;
    }
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
 * Horizontal flow: EMPTY slots are dropped, the rest distribute
 * space-evenly across the bar. If two widgets contend for space, they
 * negotiate: each may shrink toward an equal split, but no further than
 * its natural width.
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

  SidebarRenderSpec spec =
      SidebarWidgets_makeRenderSpec(r->widgetMode, contentWidth, true);

  int totalWidth = 0;
  for (int i = 0; i < visibleCount; i++) {
    sizes[i] = SidebarWidgets_measure(&widgets[i], &spec);
    totalWidth += sizes[i].w;
  }

  if (visibleCount == 1 && sizes[0].w > contentWidth) {
    totalWidth = sizes[0].w = contentWidth;
  } else if (visibleCount == 2) {
    int available = contentWidth - HBAR_WIDGET_GAP;

    if (available < 0) {
      available = 0;
    }

    if (sizes[0].w + sizes[1].w > available) {
      int leftWidth = available / 2;
      int rightWidth = available - leftWidth;

      if (sizes[0].w < leftWidth) {
        leftWidth = sizes[0].w;
        rightWidth = available - leftWidth;
      } else if (sizes[1].w < rightWidth) {
        rightWidth = sizes[1].w;
        leftWidth = available - rightWidth;
      }

      sizes[0].w = leftWidth;
      sizes[1].w = rightWidth;
      totalWidth = leftWidth + rightWidth;
    }
  }

  int remaining = bounds.size.w - totalWidth;
  int gapsLeft = visibleCount + 1;
  int x = 0;

  for (int i = 0; i < visibleCount; i++) {
    int gap = remaining / gapsLeft;
    remaining -= gap;
    gapsLeft--;
    x += gap;
    GRect frame = GRect(x, 0, sizes[i].w, bounds.size.h);
    SidebarWidgets_draw(ctx, &widgets[i], frame, &spec);
    x += sizes[i].w;
  }
}

static void region_render(SidebarRegion *r, Layer *l, GContext *ctx) {
  if (r->edge == REGION_EDGE_NONE) {
    return;
  }

  graphics_context_set_fill_color(ctx, settings.sidebarColor);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);
  graphics_context_set_text_color(ctx, settings.sidebarTextColor);

  if (r->edge == REGION_EDGE_TOP || r->edge == REGION_EDGE_BOTTOM) {
    render_horizontal_flow(r, l, ctx);
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

  draw_round_arc(ctx, bgBounds, vRegion.slots[0], 7);
}

static void updateRoundSidebarRight(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.size.h / -2,
                         bounds.size.h * 2, bounds.size.h * 2);

  draw_round_arc(ctx, bgBounds, vRegion.slots[1], 3);
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
