
#pragma once
#include "pebble.h"

// -----------------------------------------------------------------------------
// Fixed point graphics context.
// -----------------------------------------------------------------------------

#define CHECK(obj) checkObject(obj, #obj)
bool checkObject(void* obj, const char* objname);

typedef int32_t fixed_t;
struct FFont;
typedef struct FFont FFont;

// Defines the fixed point conversions
#define FIXED_POINT_SHIFT 4
#define FIXED_POINT_SCALE 16
#define INT_TO_FIXED(a) ((a) * FIXED_POINT_SCALE)
#define FIXED_TO_INT(a) ((a) / FIXED_POINT_SCALE)
#define FIXED_MULTIPLY(a, b) (((a) * (b)) / FIXED_POINT_SCALE)
#define FIX1 FIXED_POINT_SCALE

typedef struct FPoint {
    fixed_t x;
    fixed_t y;
} FPoint;
#define FPoint(x, y) ((FPoint){(x), (y)})
#define FPointI(x, y) ((FPoint){INT_TO_FIXED(x), INT_TO_FIXED(y)})
#define FPointZero FPoint(0, 0)
#define FPointOne FPoint(1, 1)

static inline bool fpoint_equal(const FPoint* const a, const FPoint* const b) {
	return a->x == b->x && a->y == b->y;
}

static inline FPoint g2fpoint(GPoint gpoint) {
	return FPoint(INT_TO_FIXED(gpoint.x), INT_TO_FIXED(gpoint.y));
}

static inline GPoint f2gpoint(FPoint fpoint) {
	return GPoint(FIXED_TO_INT(fpoint.x), FIXED_TO_INT(fpoint.y));
}

static inline FPoint fpoint_add(FPoint a, FPoint b) {
	return (FPoint){a.x + b.x, a.y + b.y};
}

static inline GPoint gpoint_add(GPoint a, GPoint b) {
	return (GPoint){a.x + b.x, a.y + b.y};
}

typedef struct FSize {
    fixed_t w;
    fixed_t h;
} FSize;

typedef struct FRect {
    FPoint origin;
    FSize size;
} FRect;

struct FEdgeRec;
struct FCircleRec;

typedef struct FContext {
	GContext* gctx;
	GBitmap* flag_buffer;
    GRect flag_bounds;
#ifdef PBL_COLOR
    /* Deferred antialiased fill state (vendored change, see fctx.c):
     * geometry is recorded during the fill and rasterized at
     * fctx_end_fill into a coverage buffer sized to the fill's
     * extent, instead of a framebuffer-sized flag bitmap. */
    struct FEdgeRec* edge_records;
    uint16_t edge_count;
    uint16_t edge_capacity;
    struct FCircleRec* circle_records;
    uint16_t circle_count;
    uint16_t circle_capacity;
    bool record_overflow;
#endif
	FPoint extent_min;
	FPoint extent_max;
    FPoint path_init_point;
    FPoint path_cur_point;
    FPoint transform_pivot;
    FPoint transform_offset;
    FPoint transform_scale_from;
	FPoint transform_scale_to;
    fixed_t transform_rotation;
	fixed_t subpixel_adjust;

    GColor fill_color;
	int16_t color_bias;
} FContext;

void fctx_set_fill_color(FContext* fctx, GColor c);
void fctx_set_color_bias(FContext* fctx, int16_t bias);
void fctx_set_pivot(FContext* fctx, FPoint pivot);
void fctx_set_offset(FContext* fctx, FPoint offset);
void fctx_set_scale(FContext* fctx, FPoint scale_from, FPoint scale_to);
void fctx_set_rotation(FContext* fctx, uint32_t rotation);

void fctx_transform_points(FContext* fctx, uint16_t pcount, FPoint* ppoints, FPoint* tpoints, FPoint advance);

void fctx_move_to   (FContext* fctx, FPoint p);
void fctx_line_to   (FContext* fctx, FPoint p);
void fctx_curve_to  (FContext* fctx, FPoint cp0, FPoint cp1, FPoint p);
void fctx_close_path(FContext* fctx);
void fctx_draw_path (FContext* fctx, FPoint* points, uint32_t num_points);
void fctx_draw_path_with_buffer(FContext* fctx, FPoint* points, FPoint* buffer, uint32_t num_points);

typedef void (*fctx_init_context_func)(FContext* fctx, GContext* gctx);
typedef void (*fctx_plot_edge_func)(FContext* fctx, FPoint* a, FPoint* b);
typedef void (*fctx_plot_circle_func)(FContext* fctx, const FPoint* c, fixed_t r);
typedef void (*fctx_end_fill_func)(FContext* fctx);

extern fctx_init_context_func fctx_init_context;
extern void fctx_begin_fill(FContext* fctx);
extern fctx_plot_edge_func fctx_plot_edge;
extern fctx_plot_circle_func fctx_plot_circle;
extern fctx_end_fill_func fctx_end_fill;
extern void fctx_deinit_context(FContext* fctx);

#ifdef PBL_COLOR
void fctx_enable_aa(bool enable);
bool fctx_is_aa_enabled();
#endif

// -----------------------------------------------------------------------------
// Compiled SVG path drawing.
// -----------------------------------------------------------------------------

typedef int16_t fixed16_t;
typedef struct {
	uint16_t code;
	fixed16_t params[];
} FPathDrawCommand;

void fctx_draw_commands(FContext* fctx, FPoint advance, void* path_data, uint16_t length);

// -----------------------------------------------------------------------------
// Text drawing.
// -----------------------------------------------------------------------------

typedef enum {
	FTextAnchorBaseline = 0,
    FTextAnchorCapMiddle,
	FTextAnchorMiddle,
	FTextAnchorTop,
    FTextAnchorCapTop,
	FTextAnchorBottom
} FTextAnchor;

void fctx_set_text_em_height(FContext* fctx, FFont* font, int16_t pixels);
void fctx_set_text_cap_height(FContext* fctx, FFont* font, int16_t pixels);
fixed_t fctx_string_width(FContext* fctx, const char* text, FFont* font);
void fctx_draw_string(FContext* fctx, const char* text, FFont* font, GTextAlignment alignment, FTextAnchor anchor);
