
#include "fctx.h"
#include "ffont.h"
#include <stdlib.h>
#include "pebble-utf8.h"

/*
 * Credit where credit is due:
 *
 * The functions fceil, floorDivMod, edge_init, and edge_step
 * are derived from Chris Hecker's "Perspective Texture Mapping"
 * series of articles in Game Developer Magazine (1995).  See
 * http://chrishecker.com/Miscellaneous_Technical_Articles
 *
 * The functions fpath_plot_edge_aa and fpath_end_fill_aa are derived
 * from:
 *   Scanline edge-flag algorithm for antialiasing
 *   Copyright (c) 2005-2007 Kiia Kallio <kkallio@uiah.fi>
 *   http://mlab.uiah.fi/~kkallio/antialiasing/
 *
 * The Edge Flag algorithm as used in both the black & white and
 * antialiased rendering functions here was presented by Bryan D. Ackland
 * and Neil H. Weste in "The Edge Flag Algorithm-A Fill Method for
 * Raster Scan Displays" (January 1981).
 *
 * The function countBits is Brian Kernighan's alorithm as presented
 * on Sean Eron Anderson's Bit Twiddling Hacks page at
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 * The bezier function is derived from Łukasz Zalewski's blog post
 * "Bezier Curves and GPaths on Pebble"
 * https://developer.getpebble.com//blog/2015/02/13/Bezier-Curves-And-GPaths/
 *
 */

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

bool checkObject(void* obj, const char* objname) {
    if (!obj) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "NULL %s", objname);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------
// Drawing support that is shared between BW and AA.
// --------------------------------------------------------------------------

void floorDivMod(int32_t numerator, int32_t denominator, int32_t* floor, int32_t* mod ) {
    Assert(denominator > 0); // we assume it's positive
    if (numerator >= 0) {
        // positive case, C is okay
        *floor = numerator / denominator;
        *mod = numerator % denominator;
    } else {
        // numerator is negative, do the right thing
        *floor = -((-numerator) / denominator);
        *mod = (-numerator) % denominator;
        if (*mod) {
            // there is a remainder
            --*floor;
            *mod = denominator - *mod;
        }
    }
}

typedef struct Edge {
    int32_t x;
    int32_t xStep;
    int32_t numerator;
    int32_t denominator;
    int32_t errorTerm; // DDA info for x
    int32_t y;         // current y
    int32_t height;    // vertical count
} Edge;

int32_t edge_step(Edge* e) {
    e->x += e->xStep;
    ++e->y;
    --e->height;

    e->errorTerm += e->numerator;
    if (e->errorTerm >= e->denominator) {
        ++e->x;
        e->errorTerm -= e->denominator;
    }
    return e->height;
}

void fctx_begin_fill(FContext* fctx) {

    /* the AA renderer has no flag buffer, so seed the extent from the
     * screen bounds recorded at init instead of the buffer's bounds */
    GRect bounds = fctx->flag_bounds;
    fctx->extent_max.x = INT_TO_FIXED(bounds.origin.x);
    fctx->extent_max.y = INT_TO_FIXED(bounds.origin.y);
    fctx->extent_min.x = INT_TO_FIXED(bounds.origin.x + bounds.size.w);
    fctx->extent_min.y = INT_TO_FIXED(bounds.origin.y + bounds.size.h);

    fctx->path_init_point.x = 0;
    fctx->path_init_point.y = 0;
    fctx->path_cur_point.x = 0;
    fctx->path_cur_point.y = 0;
}

void fctx_deinit_context(FContext* fctx) {
    if (fctx->gctx) {
        if (fctx->flag_buffer) {
            gbitmap_destroy(fctx->flag_buffer);
            fctx->flag_buffer = NULL;
        }
#ifdef PBL_COLOR
        free(fctx->edge_records);
        fctx->edge_records = NULL;
        fctx->edge_count = fctx->edge_capacity = 0;
        free(fctx->circle_records);
        fctx->circle_records = NULL;
        fctx->circle_count = fctx->circle_capacity = 0;
#endif
        fctx->gctx = NULL;
    }
}

void fctx_set_fill_color(FContext* fctx, GColor c) {
    fctx->fill_color = c;
}

void fctx_set_color_bias(FContext* fctx, int16_t bias) {
    fctx->color_bias = bias;
}

void fctx_set_pivot(FContext* fctx, FPoint pivot) {
    fctx->transform_pivot = pivot;
}

void fctx_set_offset(FContext* fctx, FPoint offset) {
    fctx->transform_offset = offset;
}

void fctx_set_scale(FContext* fctx, FPoint scale_from, FPoint scale_to) {
    fctx->transform_scale_from = scale_from;
    fctx->transform_scale_to = scale_to;
}

void fctx_set_rotation(FContext* fctx, uint32_t rotation) {
    fctx->transform_rotation = rotation;
}

// --------------------------------------------------------------------------
// BW - black and white drawing with 1 bit-per-pixel flag buffer.
// --------------------------------------------------------------------------

int32_t fceil(fixed_t value) {
    int32_t returnValue;
    int32_t numerator = value - 1 + FIXED_POINT_SCALE;
    if (numerator >= 0) {
        returnValue = numerator / FIXED_POINT_SCALE;
    } else {
        // deal with negative numerators correctly
        returnValue = -((-numerator) / FIXED_POINT_SCALE);
        returnValue -= ((-numerator) % FIXED_POINT_SCALE) ? 1 : 0;
    }
    return returnValue;
}

void edge_init(Edge* e, FPoint* top, FPoint* bottom) {

    e->y = fceil(top->y);
    int32_t yEnd = fceil(bottom->y);
    e->height = yEnd - e->y;
    if (e->height)    {
        int32_t dN = bottom->y - top->y;
        int32_t dM = bottom->x - top->x;
        int32_t initialNumerator = dM * 16 * e->y - dM * top->y +
        dN * top->x - 1 + dN * 16;
        floorDivMod(initialNumerator, dN*16, &e->x, &e->errorTerm);
        floorDivMod(dM*16, dN*16, &e->xStep, &e->numerator);
        e->denominator = dN*16;
    }
}

void fctx_init_context_bw(FContext* fctx, GContext* gctx) {

    GBitmap* frameBuffer = graphics_capture_frame_buffer(gctx);
    if (frameBuffer) {
        fctx->flag_bounds = gbitmap_get_bounds(frameBuffer);
        graphics_release_frame_buffer(gctx, frameBuffer);

        fctx->flag_buffer = gbitmap_create_blank(fctx->flag_bounds.size, GBitmapFormat1Bit);
        CHECK(fctx->flag_buffer);

#ifdef PBL_COLOR
        /* the context is typically stack-allocated; make sure deinit
         * never sees uninitialized record pointers */
        fctx->edge_records = NULL;
        fctx->edge_count = fctx->edge_capacity = 0;
        fctx->circle_records = NULL;
        fctx->circle_count = fctx->circle_capacity = 0;
        fctx->record_overflow = false;
#endif

        fctx->gctx = gctx;
        fctx->subpixel_adjust = -FIXED_POINT_SCALE / 2;
        fctx->transform_pivot = FPointZero;
        fctx->transform_offset = FPointZero;
        fctx->transform_scale_from = FPointOne;
        fctx->transform_scale_to = FPointOne;
        fctx->transform_rotation = 0;
    }
}

void fctx_plot_edge_bw(FContext* fctx, FPoint* a, FPoint* b) {

    Edge edge;
    if (a->y > b->y) {
        edge_init(&edge, b, a);
    } else {
        edge_init(&edge, a, b);
    }

    uint8_t* data = gbitmap_get_data(fctx->flag_buffer);
    int16_t stride = gbitmap_get_bytes_per_row(fctx->flag_buffer);
    int16_t max_x = fctx->flag_bounds.size.w - 1;
    int16_t max_y = fctx->flag_bounds.size.h - 1;

    while (edge.height > 0 && edge.y < 0) {
        edge_step(&edge);
    }

    while (edge.height > 0 && edge.y <= max_y) {
        if (edge.x < 0) {
            uint8_t* p = data + edge.y * stride;
            uint8_t mask = 1;
            *p ^= mask;
        } else if (edge.x <= max_x) {
            uint8_t* p = data + edge.y * stride + edge.x / 8;
            uint8_t mask = 1 << (edge.x % 8);
            *p ^= mask;
        }
        edge_step(&edge);
    }

}

static inline void fctx_plot_point_bw(FContext* fctx, int16_t x, int16_t y) {
    int16_t max_y = fctx->flag_bounds.size.h - 1;
    if (y >= 0 && y < max_y) {
        uint8_t* data = gbitmap_get_data(fctx->flag_buffer);
        int16_t stride = gbitmap_get_bytes_per_row(fctx->flag_buffer);
        int16_t max_x = fctx->flag_bounds.size.w - 1;
        if (x < 0) {
            uint8_t* p = data + y * stride;
            uint8_t mask = 1;
            *p ^= mask;
        } else if (x <= max_x) {
            uint8_t* p = data + y * stride + x / 8;
            uint8_t mask = 1 << (x % 8);
            *p ^= mask;
        }
    }
}

void fctx_plot_circle_bw(FContext* fctx, const FPoint* fc, fixed_t fr) {

    /* Expand the bounding box of pixels drawn. */
    if ((fc->x-fr) < fctx->extent_min.x) fctx->extent_min.x = fc->x - fr;
    if ((fc->y-fr) < fctx->extent_min.y) fctx->extent_min.y = fc->y - fr;
    if ((fc->x+fr) > fctx->extent_max.x) fctx->extent_max.x = fc->x + fr;
    if ((fc->y+fr) > fctx->extent_max.y) fctx->extent_max.y = fc->y + fr;

    int16_t r = FIXED_TO_INT(fr);
    int16_t cx = FIXED_TO_INT(fc->x);
    int16_t cy = FIXED_TO_INT(fc->y);

    fixed_t x = r - 1;
    fixed_t y = 0;
    fixed_t E = 1 - 2*r;
    while (x >= y) {

        fctx_plot_point_bw(fctx, cx-x-1, cy+y);
        fctx_plot_point_bw(fctx, cx+x+1, cy+y);
        fctx_plot_point_bw(fctx, cx-x-1, cy-y-1);
        fctx_plot_point_bw(fctx, cx+x+1, cy-y-1);

        E += 4*y + 4;
        if (E > 0) {

            /* Only plot the diagonally reflected octants when we
             * are going to step in x (but before the step).
             * This way, we only plot the one, maximal span
             * for each reflected row.
             * Also, do not plot these octants when m==n,
             * since that would double-plot the same points, which,
             * with the edge-flag algorithm, complete erases the span!
             */
            if (x != y) {
                fctx_plot_point_bw(fctx, cx-y-1, cy+x);
                fctx_plot_point_bw(fctx, cx+y+1, cy+x);
                fctx_plot_point_bw(fctx, cx-y-1, cy-x-1);
                fctx_plot_point_bw(fctx, cx+y+1, cy-x-1);
            }

            E += -4*x;
            --x;
        }
        ++y;
    }
}

void fctx_end_fill_bw(FContext* fctx) {

    uint8_t color;
#ifdef PBL_COLOR
    color = fctx->fill_color.argb;
#else
    uint8_t gray = 0;
    if (gcolor_equal(fctx->fill_color, GColorWhite)) {
        color = 0xff;
    } else if (gcolor_equal(fctx->fill_color, GColorBlack)) {
        color = 0x00;
    } else {
        gray = 0b01010101;
        color = gray;
    }
#endif

    int16_t rowMin = FIXED_TO_INT(fctx->extent_min.y);
    int16_t rowMax = FIXED_TO_INT(fctx->extent_max.y);
    int16_t colMin = FIXED_TO_INT(fctx->extent_min.x);
    int16_t colMax = FIXED_TO_INT(fctx->extent_max.x);

    if (rowMin < 0) rowMin = 0;
    if (rowMax >= fctx->flag_bounds.size.h) rowMax = fctx->flag_bounds.size.h - 1;
    if (colMin < 0) colMin = 0;
    if (colMax >= fctx->flag_bounds.size.w) colMax = fctx->flag_bounds.size.w - 1;

    GBitmap* fb = graphics_capture_frame_buffer(fctx->gctx);

    uint8_t* dest;
    uint8_t* src;
    uint8_t mask;
    int16_t col, row;

    for (row = rowMin; row <= rowMax; ++row) {
#ifdef PBL_BW
        if (gray) {
            if (row & 1) {
                color = gray;
            } else {
                color = ~gray;
            }
        }
#endif
        GBitmapDataRowInfo fbRowInfo = gbitmap_get_data_row_info(fb, row);
        GBitmapDataRowInfo flagRowInfo = gbitmap_get_data_row_info(fctx->flag_buffer, row);
#if defined(PBL_ROUND)
        int16_t spanMin = (fbRowInfo.min_x > colMin) ? fbRowInfo.min_x : colMin;
        int16_t spanMax = (fbRowInfo.max_x < colMax) ? fbRowInfo.max_x : colMax;
#endif
        bool inside = false;
        for (col = colMin; col <= colMax; ++col) {

#ifdef PBL_COLOR
            dest = fbRowInfo.data + col;
#else
            dest = fbRowInfo.data + col / 8;
#endif
            src = flagRowInfo.data + col / 8;
            mask = 1 << (col % 8);
            if (*src & mask) {
                inside = !inside;
            }
            *src &= ~mask;
            if (inside) {
#if defined(PBL_ROUND)
                if (col >= spanMin && col <= spanMax) {
                    *dest = color;
                }
#elif defined(PBL_COLOR)
                *dest = color;
#else
                *dest = (color & mask) | (*dest & ~mask);
#endif
            }
        }
        if (col < flagRowInfo.max_x) {
            src = flagRowInfo.data + col / 8;
            mask = 1 << (col % 8);
            *src &= ~mask;
        }
    }

    graphics_release_frame_buffer(fctx->gctx, fb);

}

// --------------------------------------------------------------------------
// AA - anti-aliased drawing with 8 bit-per-pixel flag buffer.
// --------------------------------------------------------------------------

#ifdef PBL_COLOR

#define SUBPIXEL_COUNT 8
#define SUBPIXEL_SHIFT 3

#define FIXED_POINT_SHIFT_AA 1
#define FIXED_POINT_SCALE_AA 2
#define INT_TO_FIXED_AA(a) ((a) * FIXED_POINT_SCALE)
#define FIXED_TO_INT_AA(a) ((a) / FIXED_POINT_SCALE)
#define FIXED_MULTIPLY_AA(a, b) (((a) * (b)) / FIXED_POINT_SCALE_AA)

int32_t fceil_aa(fixed_t value) {
    int32_t returnValue;
    int32_t numerator = value - 1 + FIXED_POINT_SCALE_AA;
    if (numerator >= 0) {
        returnValue = numerator / FIXED_POINT_SCALE_AA;
    } else {
        // deal with negative numerators correctly
        returnValue = -((-numerator) / FIXED_POINT_SCALE_AA);
        returnValue -= ((-numerator) % FIXED_POINT_SCALE_AA) ? 1 : 0;
    }
    return returnValue;
}

/*
 * FPoint is at a scale factor of 16.  The anti-aliased scan conversion needs
 * to address 8x8 subpixels, so if we treat the FPoint coordinates as having
 * a scale factor of 2, then we should scan in sub-pixel coordinates, with
 * sub-sub-pixel correct endpoints!  Fukn shweet.
 */
void edge_init_aa(Edge* e, FPoint* top, FPoint* bottom) {
    static const int32_t F = 2;
    e->y = fceil_aa(top->y);
    int32_t yEnd = fceil_aa(bottom->y);
    e->height = yEnd - e->y;
    if (e->height)    {
        int32_t dN = bottom->y - top->y;
        int32_t dM = bottom->x - top->x;
        int32_t initialNumerator = dM * F * e->y - dM * top->y +
        dN * top->x - 1 + dN * F;
        floorDivMod(initialNumerator, dN*F, &e->x, &e->errorTerm);
        floorDivMod(dM*F, dN*F, &e->xStep, &e->numerator);
        e->denominator = dN*F;
    }
}

/*
 * Vendored change: deferred, extent-bounded antialiased rendering.
 *
 * The stock renderer allocated a framebuffer-sized 8-bit flag bitmap for
 * every context (24KB on a 144x168 watch, 45KB on 200x228), which starved
 * the heap.  Instead, fills now record their (already transformed and
 * flattened) geometry into small growable arrays, and fctx_end_fill
 * rasterizes them with the same edge-flag algorithm into a coverage
 * buffer that is only as large as the fill's bounding box.
 *
 * Coordinates are stored as int16_t in 1/16px fixed point: +/-2048px of
 * addressable space, ample for every Pebble display.
 */

typedef struct FEdgeRec {
    int16_t x0, y0, x1, y1;
} FEdgeRec;

typedef struct FCircleRec {
    int16_t cx, cy, r;
} FCircleRec;

/* Pixel-addressed 8-bit coverage buffer over the fill's bounding box.
 * Each byte holds the 8 subpixel-row crossing flags for one pixel. */
typedef struct FCoverage {
    uint8_t* data;
    int16_t origin_x; // pixel coords of data[0] in screen space
    int16_t origin_y;
    int16_t w;
    int16_t h;
} FCoverage;

void fctx_init_context_aa(FContext* fctx, GContext* gctx) {

    GBitmap* frameBuffer = graphics_capture_frame_buffer(gctx);
    if (frameBuffer) {
        fctx->flag_bounds = gbitmap_get_bounds(frameBuffer);
        graphics_release_frame_buffer(gctx, frameBuffer);
        fctx->gctx = gctx;
        fctx->flag_buffer = NULL; // AA mode no longer uses a flag bitmap
        fctx->edge_records = NULL;
        fctx->edge_count = fctx->edge_capacity = 0;
        fctx->circle_records = NULL;
        fctx->circle_count = fctx->circle_capacity = 0;
        fctx->record_overflow = false;
        fctx->fill_color = GColorWhite;
        fctx->color_bias = 0;
        fctx->subpixel_adjust = -1;
        fctx->transform_pivot = FPointZero;
        fctx->transform_offset = FPointZero;
        fctx->transform_scale_from = FPointOne;
        fctx->transform_scale_to = FPointOne;
        fctx->transform_rotation = 0;
    }
}

static const int32_t k_sampling_offsets[SUBPIXEL_COUNT] = {
    2, 7, 4, 1, 6, 3, 0, 5 // 1/8ths
};

void fctx_plot_edge_aa(FContext* fctx, FPoint* a, FPoint* b) {

    if (a->y == b->y) {
        return; // horizontal edges never cross a scanline
    }
    if (fctx->edge_count == fctx->edge_capacity) {
        uint16_t capacity = fctx->edge_capacity ? fctx->edge_capacity * 2 : 128;
        FEdgeRec* grown = realloc(fctx->edge_records, capacity * sizeof(FEdgeRec));
        if (!grown) {
            fctx->record_overflow = true;
            return;
        }
        fctx->edge_records = grown;
        fctx->edge_capacity = capacity;
    }
    FEdgeRec* rec = fctx->edge_records + fctx->edge_count++;
    rec->x0 = a->x;
    rec->y0 = a->y;
    rec->x1 = b->x;
    rec->y1 = b->y;
}

void fctx_plot_circle_aa(FContext* fctx, const FPoint* c, fixed_t r) {

    /* Expand the bounding box of pixels drawn. */
    if ((c->x-r) < fctx->extent_min.x) fctx->extent_min.x = c->x - r;
    if ((c->y-r) < fctx->extent_min.y) fctx->extent_min.y = c->y - r;
    if ((c->x+r) > fctx->extent_max.x) fctx->extent_max.x = c->x + r;
    if ((c->y+r) > fctx->extent_max.y) fctx->extent_max.y = c->y + r;

    if (fctx->circle_count == fctx->circle_capacity) {
        uint16_t capacity = fctx->circle_capacity ? fctx->circle_capacity * 2 : 8;
        FCircleRec* grown = realloc(fctx->circle_records, capacity * sizeof(FCircleRec));
        if (!grown) {
            fctx->record_overflow = true;
            return;
        }
        fctx->circle_records = grown;
        fctx->circle_capacity = capacity;
    }
    FCircleRec* rec = fctx->circle_records + fctx->circle_count++;
    rec->cx = c->x;
    rec->cy = c->y;
    rec->r = r;
}

static void fctx_cov_plot_edge(FCoverage* cov, FPoint* a, FPoint* b) {

    Edge edge;
    if (a->y > b->y) {
        edge_init_aa(&edge, b, a);
    } else {
        edge_init_aa(&edge, a, b);
    }

    /* subpixel rows spanned by the coverage buffer; origin is never
     * negative because the bounding box is clipped to the screen */
    int32_t min_y = cov->origin_y * SUBPIXEL_COUNT;
    int32_t max_y = (cov->origin_y + cov->h) * SUBPIXEL_COUNT - 1;

    while (edge.height > 0 && edge.y < min_y) {
        edge_step(&edge);
    }

    while (edge.height > 0 && edge.y <= max_y) {
        int32_t ySub = edge.y & (SUBPIXEL_COUNT - 1);
        uint8_t mask = 1 << ySub;
        int32_t pixelX = (edge.x + k_sampling_offsets[ySub]) / SUBPIXEL_COUNT - cov->origin_x;
        int32_t pixelY = edge.y / SUBPIXEL_COUNT - cov->origin_y;
        uint8_t* row = cov->data + pixelY * cov->w;
        if (pixelX < 0) {
            /* crossings left of the box still toggle parity for it */
            row[0] ^= mask;
        } else if (pixelX < cov->w) {
            row[pixelX] ^= mask;
        }
        edge_step(&edge);
    }
}

static inline void fctx_cov_plot_point(FCoverage* cov, fixed_t x, fixed_t y) {
    int32_t ySub = y & (SUBPIXEL_COUNT - 1);
    uint8_t mask = 1 << ySub;
    int32_t pixelX = (x + k_sampling_offsets[ySub]) / SUBPIXEL_COUNT - cov->origin_x;
    int32_t pixelY = y / SUBPIXEL_COUNT - cov->origin_y;

    if (pixelY >= 0 && pixelY < cov->h) {
        uint8_t* row = cov->data + pixelY * cov->w;
        if (pixelX < 0) {
            row[0] ^= mask;
        } else if (pixelX < cov->w) {
            row[pixelX] ^= mask;
        }
    }
}

static void fctx_cov_plot_circle(FCoverage* cov, fixed_t cx, fixed_t cy, fixed_t r) {

    /* Throw away the extra bit of fixed point precision and
     * work directly in subpixels.
     */
    r = r / 2;
    cx = cx / 2;
    cy = cy / 2;

    fixed_t m = r - 1;
    fixed_t n = 0;
    fixed_t E = 1 - 2*r;
    while (m >= n) {

        fctx_cov_plot_point(cov, cx-m-1, cy+n);
        fctx_cov_plot_point(cov, cx+m+1, cy+n);
        fctx_cov_plot_point(cov, cx-m-1, cy-n-1);
        fctx_cov_plot_point(cov, cx+m+1, cy-n-1);

        E += 4*n + 4;
        if (E > 0) {

            /* Only plot the diagonally reflected octants when we
             * are going to step in x (but before the step).
             * This way, we only plot the one, maximal span
             * for each reflected row.
             * Also, do not plot these octants when m==n,
             * since that would double-plot the same points, which,
             * with the edge-flag algorithm, complete erases the span!
             */
            if (m != n) {
                fctx_cov_plot_point(cov, cx-n-1, cy+m);
                fctx_cov_plot_point(cov, cx+n+1, cy+m);
                fctx_cov_plot_point(cov, cx-n-1, cy-m-1);
                fctx_cov_plot_point(cov, cx+n+1, cy-m-1);
            }

            E += -4*m;
            --m;
        }
        ++n;
    }
}

// count the number of bits set in v
uint8_t countBits(uint8_t v) {
    unsigned int c; // c accumulates the total bits set in v
    for (c = 0; v; c++)    {
        v &= v - 1; // clear the least significant bit set
    }
    return c;
}

static inline int8_t clamp8(int8_t val, int8_t min, int8_t max) {
    if (val <= min) return min;
    if (val >= max) return max;
    return val;
}

void fctx_end_fill_aa(FContext* fctx) {

    uint16_t edge_count = fctx->edge_count;
    uint16_t circle_count = fctx->circle_count;
    fctx->edge_count = 0;
    fctx->circle_count = 0;

    if (fctx->record_overflow) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "fctx: fill dropped, no heap for records");
        fctx->record_overflow = false;
        return;
    }
    if (edge_count == 0 && circle_count == 0) {
        return;
    }

    /* Pixel bounding box of the fill: padded by one pixel because
     * subpixel sampling can push a crossing just past the extent,
     * then clipped to the screen. */
    int16_t colMin = FIXED_TO_INT(fctx->extent_min.x) - 1;
    int16_t rowMin = FIXED_TO_INT(fctx->extent_min.y) - 1;
    int16_t colMax = FIXED_TO_INT(fctx->extent_max.x) + 1;
    int16_t rowMax = FIXED_TO_INT(fctx->extent_max.y) + 1;

    if (colMin < 0) colMin = 0;
    if (rowMin < 0) rowMin = 0;
    if (colMax >= fctx->flag_bounds.size.w) colMax = fctx->flag_bounds.size.w - 1;
    if (rowMax >= fctx->flag_bounds.size.h) rowMax = fctx->flag_bounds.size.h - 1;

    FCoverage cov;
    cov.origin_x = colMin;
    cov.origin_y = rowMin;
    cov.w = colMax - colMin + 1;
    cov.h = rowMax - rowMin + 1;
    if (cov.w <= 0 || cov.h <= 0) {
        return; // fill lies entirely off screen
    }

    cov.data = malloc((size_t)cov.w * cov.h);
    if (!cov.data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "fctx: fill dropped, no heap for %dx%d coverage", cov.w, cov.h);
        return;
    }
    memset(cov.data, 0, (size_t)cov.w * cov.h);

    for (uint16_t k = 0; k < edge_count; ++k) {
        FEdgeRec* rec = fctx->edge_records + k;
        FPoint a = {rec->x0, rec->y0};
        FPoint b = {rec->x1, rec->y1};
        fctx_cov_plot_edge(&cov, &a, &b);
    }
    for (uint16_t k = 0; k < circle_count; ++k) {
        FCircleRec* rec = fctx->circle_records + k;
        fctx_cov_plot_circle(&cov, rec->cx, rec->cy, rec->r);
    }

    GBitmap* fb = graphics_capture_frame_buffer(fctx->gctx);

    GColor8 d;
    GColor8 s = fctx->fill_color;
    int16_t bias = fctx->color_bias;
    for (int16_t row = rowMin; row <= rowMax; ++row) {
        GBitmapDataRowInfo fbRowInfo = gbitmap_get_data_row_info(fb, row);
        uint8_t* src = cov.data + (row - rowMin) * cov.w;

        /* parity accumulates across the whole box so that spans running
         * under a round display's clipped columns stay correct */
        uint8_t mask = 0;
        for (int16_t col = 0; col < cov.w; ++col) {
            mask ^= src[col];
            int16_t x = colMin + col;
            if (x < fbRowInfo.min_x || x > fbRowInfo.max_x) {
                continue;
            }
            uint8_t a = clamp8(countBits(mask) + bias, 0, 8);
            if (a) {
                uint8_t* dest = fbRowInfo.data + x;
                d.argb = *dest;
                d.r = (s.r*a + d.r*(8 - a) + 4) / 8;
                d.g = (s.g*a + d.g*(8 - a) + 4) / 8;
                d.b = (s.b*a + d.b*(8 - a) + 4) / 8;
                *dest = d.argb;
            }
        }
    }

    graphics_release_frame_buffer(fctx->gctx, fb);
    free(cov.data);
}

// Initialize for Anti-Aliased rendering.
fctx_init_context_func   fctx_init_context   = &fctx_init_context_aa;
fctx_plot_edge_func      fctx_plot_edge      = &fctx_plot_edge_aa;
fctx_plot_circle_func    fctx_plot_circle    = &fctx_plot_circle_aa;
fctx_end_fill_func       fctx_end_fill       = &fctx_end_fill_aa;

void fctx_enable_aa(bool enable) {
    if (enable) {
        fctx_init_context   = &fctx_init_context_aa;
        fctx_plot_edge      = &fctx_plot_edge_aa;
        fctx_plot_circle    = &fctx_plot_circle_aa;
        fctx_end_fill       = &fctx_end_fill_aa;
    } else {
        fctx_init_context   = &fctx_init_context_bw;
        fctx_plot_edge      = &fctx_plot_edge_bw;
        fctx_plot_circle    = &fctx_plot_circle_bw;
        fctx_end_fill       = &fctx_end_fill_bw;
    }
}

bool fctx_is_aa_enabled() {
    return fctx_init_context == &fctx_init_context_aa;
}

#else

// Initialize for Black & White rendering.
fctx_init_context_func   fctx_init_context   = &fctx_init_context_bw;
fctx_plot_edge_func      fctx_plot_edge      = &fctx_plot_edge_bw;
fctx_plot_circle_func    fctx_plot_circle    = &fctx_plot_circle_bw;
fctx_end_fill_func       fctx_end_fill       = &fctx_end_fill_bw;

#endif

// --------------------------------------------------------------------------
// Transformed Drawing
// --------------------------------------------------------------------------

static void bezier(FContext* fctx,
            fixed_t x1, fixed_t y1,
            fixed_t x2, fixed_t y2,
            fixed_t x3, fixed_t y3,
            fixed_t x4, fixed_t y4) {

    // Angle below which we're not going to process with recursion
    static const int32_t max_angle_tolerance = (TRIG_MAX_ANGLE / 360) * 5;

    // Calculate all the mid-points of the line segments
    fixed_t x12   = (x1 + x2) / 2;
    fixed_t y12   = (y1 + y2) / 2;
    fixed_t x23   = (x2 + x3) / 2;
    fixed_t y23   = (y2 + y3) / 2;
    fixed_t x34   = (x3 + x4) / 2;
    fixed_t y34   = (y3 + y4) / 2;
    fixed_t x123  = (x12 + x23) / 2;
    fixed_t y123  = (y12 + y23) / 2;
    fixed_t x234  = (x23 + x34) / 2;
    fixed_t y234  = (y23 + y34) / 2;
    fixed_t x1234 = (x123 + x234) / 2;
    fixed_t y1234 = (y123 + y234) / 2;

    // Angle Condition
    int32_t a23 = atan2_lookup((int16_t)((y3 - y2) / FIXED_POINT_SCALE),
                               (int16_t)((x3 - x2) / FIXED_POINT_SCALE));
    int32_t da1 = abs(a23 - atan2_lookup((int16_t)((y2 - y1) / FIXED_POINT_SCALE),
                                         (int16_t)((x2 - x1) / FIXED_POINT_SCALE)));
    int32_t da2 = abs(atan2_lookup((int16_t)((y4 - y3) / FIXED_POINT_SCALE),
                                   (int16_t)((x4 - x3) / FIXED_POINT_SCALE)) - a23);

    if (da1 >= TRIG_MAX_ANGLE) {
        da1 = TRIG_MAX_ANGLE - da1;
    }

    if (da2 >= TRIG_MAX_ANGLE) {
        da2 = TRIG_MAX_ANGLE - da2;
    }

    if (da1 + da2 < max_angle_tolerance) {
        // Finally we can stop the recursion
        FPoint a = {x1, y1};
        FPoint b = {x4, y4};
        fctx_plot_edge(fctx, &a, &b);
        return;
    }

    // Continue subdivision if points are being added successfully
    bezier(fctx, x1, y1, x12, y12, x123, y123, x1234, y1234);
    bezier(fctx, x1234, y1234, x234, y234, x34, y34, x4, y4);

}

void fctx_move_to_func(FContext* fctx, FPoint* params) {
    fctx->path_init_point = params[0];
    fctx->path_cur_point = params[0];
}

void fctx_line_to_func(FContext* fctx, FPoint* params) {
    fctx_plot_edge(fctx, &fctx->path_cur_point, params + 0);
    fctx->path_cur_point = params[0];
}

void fctx_curve_to_func(FContext* fctx, FPoint* params) {
    bezier(fctx,
           fctx->path_cur_point.x, fctx->path_cur_point.y,
           params[0].x, params[0].y,
           params[1].x, params[1].y,
           params[2].x, params[2].y);
    fctx->path_cur_point = params[2];
}

typedef void (*fctx_draw_cmd_func)(FContext* fctx, FPoint* params);

void fctx_transform_points(FContext* fctx, uint16_t pcount, FPoint* ppoints, FPoint* tpoints, FPoint advance) {

    int32_t c = cos_lookup(fctx->transform_rotation);
    int32_t s = sin_lookup(fctx->transform_rotation);

    /* transform the parameters */
    FPoint* src = ppoints;
    FPoint* dst = tpoints;
    FPoint* end = dst + pcount;
    while (dst != end) {
        fixed_t x = (src->x - fctx->transform_pivot.x + advance.x) * fctx->transform_scale_to.x / fctx->transform_scale_from.x;
        fixed_t y = (src->y - fctx->transform_pivot.y + advance.y) * fctx->transform_scale_to.y / fctx->transform_scale_from.y;
        dst->x = (x * c / TRIG_MAX_RATIO) - (y * s / TRIG_MAX_RATIO);
        dst->y = (x * s / TRIG_MAX_RATIO) + (y * c / TRIG_MAX_RATIO);
        dst->x += fctx->transform_offset.x + fctx->subpixel_adjust;
        dst->y += fctx->transform_offset.y + fctx->subpixel_adjust;

        // grow a bounding box around the points visited.
        if (dst->x < fctx->extent_min.x) fctx->extent_min.x = dst->x;
        if (dst->y < fctx->extent_min.y) fctx->extent_min.y = dst->y;
        if (dst->x > fctx->extent_max.x) fctx->extent_max.x = dst->x;
        if (dst->y > fctx->extent_max.y) fctx->extent_max.y = dst->y;

        ++src;
        ++dst;
    }
}

static void exec_draw_func(FContext* fctx, FPoint advance, fctx_draw_cmd_func func, FPoint* ppoints, uint16_t pcount) {
    FPoint tpoints[3];
    fctx_transform_points(fctx, pcount, ppoints, tpoints, advance);
    func(fctx, tpoints);
}

void fctx_move_to(FContext* fctx, FPoint p) {
    exec_draw_func(fctx, FPointZero, fctx_move_to_func, &p, 1);
}

void fctx_line_to(FContext* fctx, FPoint p) {
    exec_draw_func(fctx, FPointZero, fctx_line_to_func, &p, 1);
}

void fctx_curve_to(FContext* fctx, FPoint cp0, FPoint cp1, FPoint p) {
    FPoint points[3];
    points[0] = cp0;
    points[1] = cp1;
    points[2] = p;
    exec_draw_func(fctx, FPointZero, fctx_curve_to_func, points, 3);
}

void fctx_close_path(FContext* fctx) {
    fctx_plot_edge(fctx, &fctx->path_cur_point, &fctx->path_init_point);
    fctx->path_cur_point = fctx->path_init_point;
}

void fctx_draw_path_with_buffer(FContext* fctx, FPoint* points, FPoint* buffer, uint32_t num_points) {

    fctx_transform_points(fctx, num_points, points, buffer, FPointZero);
    for (uint32_t k = 0; k < num_points; ++k) {
        fctx_plot_edge(fctx, buffer+k, buffer+((k+1) % num_points));
    }
}

void fctx_draw_path(FContext* fctx, FPoint* points, uint32_t num_points) {

    FPoint* buffer = (FPoint*)malloc(num_points * sizeof(FPoint));
    if (buffer) {
        fctx_draw_path_with_buffer(fctx, points, buffer, num_points);
        free(buffer);
    }
}

void fctx_draw_commands(FContext* fctx, FPoint advance, void* path_data, uint16_t length) {

    fctx_draw_cmd_func func;
    uint16_t pcount;
    FPoint initpt = {0, 0};
    FPoint curpt = {0, 0};
    FPoint ctrlpt = {0, 0};
    FPoint ppoints[3];

    void* path_data_end = path_data + length;
    while (path_data < path_data_end) {

        /* choose the draw function and parameter count. */
        FPathDrawCommand* cmd = (FPathDrawCommand*)path_data;
        fixed16_t* param = cmd->params;
        switch (cmd->code) {
            case 'M': // "moveto"
                func = fctx_move_to_func;
                pcount = 1;
                ppoints[0].x = *param++;
                ppoints[0].y = *param++;
                curpt = ppoints[0];
                initpt = curpt;
                break;
            case 'Z': // "closepath"
                func = fctx_line_to_func;
                pcount = 1;
                ppoints[0] = initpt;
                curpt = ppoints[0];
                break;
            case 'L': // "lineto"
                func = fctx_line_to_func;
                pcount = 1;
                ppoints[0].x = *param++;
                ppoints[0].y = *param++;
                curpt = ppoints[0];
                break;
            case 'H': // "horizontal lineto"
                func = fctx_line_to_func;
                pcount = 1;
                ppoints[0].x = *param++;
                ppoints[0].y = curpt.y;
                curpt.x = ppoints[0].x;
                break;
            case 'V': // "vertical lineto"
                func = fctx_line_to_func;
                pcount = 1;
                ppoints[0].x = curpt.x;
                ppoints[0].y = *param++;
                curpt.y = ppoints[0].y;
                break;
            case 'C': // "cubic bezier curveto"
                func = fctx_curve_to_func;
                pcount = 3;
                ppoints[0].x = *param++;
                ppoints[0].y = *param++;
                ppoints[1].x = *param++;
                ppoints[1].y = *param++;
                ppoints[2].x = *param++;
                ppoints[2].y = *param++;
                ctrlpt = ppoints[1];
                curpt = ppoints[2];
                break;
            case 'S': // "smooth cubic bezier curveto"
                func = fctx_curve_to_func;
                pcount = 3;
                ppoints[1].x = *param++;
                ppoints[1].y = *param++;
                ppoints[2].x = *param++;
                ppoints[2].y = *param++;
                ppoints[0].x = curpt.x - ctrlpt.x + curpt.x;
                ppoints[0].y = curpt.y - ctrlpt.y + curpt.y;
                ctrlpt = ppoints[1];
                curpt = ppoints[2];
                break;
            case 'Q': // "quadratic bezier curveto"
                func = fctx_curve_to_func;
                pcount = 3;
                ctrlpt.x = *param++;
                ctrlpt.y = *param++;
                ppoints[2].x = *param++;
                ppoints[2].y = *param++;
                ppoints[0].x = (curpt.x      + 2 * ctrlpt.x) / 3;
                ppoints[0].y = (curpt.y      + 2 * ctrlpt.y) / 3;
                ppoints[1].x = (ppoints[2].x + 2 * ctrlpt.x) / 3;
                ppoints[1].y = (ppoints[2].y + 2 * ctrlpt.y) / 3;
                curpt = ppoints[2];
                break;
            case 'T': // "smooth quadratic bezier curveto"
                func = fctx_curve_to_func;
                pcount = 3;
                ctrlpt.x = curpt.x - ctrlpt.x + curpt.x;
                ctrlpt.y = curpt.y - ctrlpt.y + curpt.y;
                ppoints[2].x = *param++;
                ppoints[2].y = *param++;
                ppoints[0].x = (curpt.x      + 2 * ctrlpt.x) / 3;
                ppoints[0].y = (curpt.y      + 2 * ctrlpt.y) / 3;
                ppoints[1].x = (ppoints[2].x + 2 * ctrlpt.x) / 3;
                ppoints[1].y = (ppoints[2].y + 2 * ctrlpt.y) / 3;
                curpt = ppoints[2];
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "invalid draw command %d", cmd->code);
                return;
        }

        /* advance to next draw command */
        path_data = (void*)param;

        if (func) {
            exec_draw_func(fctx, advance, func, ppoints, pcount);
        }
    }
}

// --------------------------------------------------------------------------
// Text
// --------------------------------------------------------------------------

void fctx_set_text_em_height(FContext* fctx, FFont* font, int16_t pixels) {
    fctx->transform_scale_from.x = FIXED_TO_INT(font->units_per_em);
    fctx->transform_scale_from.y = -fctx->transform_scale_from.x;
    fctx->transform_scale_to.x = pixels;
    fctx->transform_scale_to.y = pixels;
}

void fctx_set_text_cap_height(FContext* fctx, FFont* font, int16_t pixels) {
    fctx->transform_scale_from.x = FIXED_TO_INT(font->cap_height);
    fctx->transform_scale_from.y = -fctx->transform_scale_from.x;
    fctx->transform_scale_to.x = pixels;
    fctx->transform_scale_to.y = pixels;
}

fixed_t fctx_string_width(FContext* fctx, const char* text, FFont* font) {
    uint16_t code_point;
    uint16_t decode_state = 0;
    fixed_t width = 0;
    for (const char* p = text; *p; ++p) {
        if (0 == utf8_decode_byte(*p, &decode_state, &code_point)) {
            FGlyph* glyph = ffont_glyph_info(font, code_point);
            if (glyph) {
                width += glyph->horiz_adv_x;
            }
        }
    }
    return width * fctx->transform_scale_to.x / fctx->transform_scale_from.x;
}

void fctx_draw_string(FContext* fctx, const char* text, FFont* font, GTextAlignment alignment, FTextAnchor anchor) {

    FPoint advance = {0, 0};
    uint16_t code_point;
    uint16_t decode_state;
    const char* p;

    if (alignment != GTextAlignmentLeft) {
        fixed_t width = 0;
        decode_state = 0;
        for (p = text; *p; ++p) {
            if (0 == utf8_decode_byte(*p, &decode_state, &code_point)) {
                FGlyph* glyph = ffont_glyph_info(font, code_point);
                if (glyph) {
                    width += glyph->horiz_adv_x;
                }
            }
        }
        if (alignment == GTextAlignmentRight) {
            advance.x = -width;
        } else /* alignment == GTextAlignmentCenter */ {
            advance.x = -width / 2;
        }
    }

    if (anchor == FTextAnchorBottom) {
        advance.y = -font->descent;
    } else if (anchor == FTextAnchorMiddle) {
        advance.y = -font->ascent / 2;
    } else if (anchor == FTextAnchorCapMiddle) {
        advance.y = -font->cap_height / 2;
    } else if (anchor == FTextAnchorTop) {
        advance.y = -font->ascent;
    } else if (anchor == FTextAnchorCapTop) {
        advance.y = -font->cap_height;
    } else /* anchor == FTextAnchorBaseline) */ {
        advance.y = 0;
    }

    decode_state = 0;
    for (p = text; *p; ++p) {
        if (0 == utf8_decode_byte(*p, &decode_state, &code_point)) {
            FGlyph* glyph = ffont_glyph_info(font, code_point);
            if (glyph) {
                void* path_data = ffont_glyph_outline(font, glyph);
                fctx_draw_commands(fctx, advance, path_data, glyph->path_data_length);
                advance.x += glyph->horiz_adv_x;
            }
        }
    }
}
