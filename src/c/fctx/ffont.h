
#pragma once
#include "fctx.h"

typedef struct __attribute__((__packed__)) FFont {
    fixed16_t units_per_em;
    fixed16_t ascent;
    fixed16_t descent;
    fixed16_t cap_height;
    uint16_t glyph_index_length;
    uint16_t glyph_table_length;
} FFont;

typedef struct __attribute__((__packed__)) FGlyphRange {
    uint16_t begin;
    uint16_t end;
} FGlyphRange;

typedef struct __attribute__((__packed__)) FGlyph {
    uint16_t path_data_offset;
    uint16_t path_data_length;
    fixed16_t horiz_adv_x;
} FGlyph;

FFont* ffont_load_from_resource_into_buffer(uint32_t resource_id, void* buffer);
FFont* ffont_create_from_resource(uint32_t resource_id);
void ffont_destroy(FFont* font);
void ffont_debug_log(FFont* font, uint8_t log_level);
FGlyph* ffont_glyph_info(FFont* font, uint16_t unicode);
void* ffont_glyph_outline(FFont* font, FGlyph* glyph);
