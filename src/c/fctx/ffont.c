
#include "ffont.h"

FFont* ffont_create_from_resource(uint32_t resource_id) {
    ResHandle rh = resource_get_handle(resource_id);
    size_t rs = resource_size(rh);
    void* buffer = malloc(rs);
    if (buffer) {
        resource_load(rh, buffer, rs);
        return (FFont*)buffer;
    }
    return NULL;
}

FFont* ffont_load_from_resource_into_buffer(uint32_t resource_id, void* buffer) {
    ResHandle rh = resource_get_handle(resource_id);
    size_t rs = resource_size(rh);
    if (buffer) {
        resource_load(rh, buffer, rs);
        return (FFont*)buffer;
    }
    return NULL;
}

FGlyphRange* ffont_glyph_index(FFont* font) {
    void* buffer = (void*)font;
    void* index = buffer + sizeof(FFont);
    return (FGlyphRange*)index;
}

FGlyph* ffont_glyph_table(FFont* font) {
    void* buffer = (void*)font;
    void* table = buffer + sizeof(FFont)
                + font->glyph_index_length * sizeof(FGlyphRange);
    return (FGlyph*)table;
}

void* ffont_path_data(FFont* font) {
    void* buffer = (void*)font;
    void* path_data = buffer + sizeof(FFont)
                    + font->glyph_index_length * sizeof(FGlyphRange)
                    + font->glyph_table_length * sizeof(FGlyph);
    return path_data;
}

FGlyph* ffont_glyph_info(FFont* font, uint16_t unicode) {
    FGlyphRange* range = ffont_glyph_index(font);
    FGlyphRange* end = range + font->glyph_index_length;
    uint16_t offset = 0;
    while (range < end) {
        if (unicode < range->begin) {
            break;
        }
        if (unicode < range->end) {
            FGlyph* table = ffont_glyph_table(font);
            FGlyph* g = table + offset + (unicode - range->begin);
            /*APP_LOG(APP_LOG_LEVEL_DEBUG, "U+%04X @ U+%04X + %02X",
                unicode, range->begin, unicode - range->begin);*/
            return g;
        }
        offset += (range->end - range->begin);
        ++range;
    }
#if 0
    APP_LOG(APP_LOG_LEVEL_WARNING, "U+%04x no glyph", unicode);
#endif
    return NULL;
}

void* ffont_glyph_outline(FFont* font, FGlyph* glyph) {
    void* path_data = ffont_path_data(font);
    return path_data + glyph->path_data_offset;
}

void ffont_debug_log(FFont* font, uint8_t log_level) {
    if (log_level >= APP_LOG_LEVEL_WARNING && font == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "font not loaded");
        return;
    }

    if (log_level >= APP_LOG_LEVEL_DEBUG_VERBOSE) {
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "so(FF):%d so(FGR):%d so(FG):%d", sizeof(FFont), sizeof(FGlyphRange), sizeof(FGlyph));
        APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "upm:%d a:%d d:%d gil:%d gtl:%d", font->units_per_em, font->ascent, font->descent, font->glyph_index_length, font->glyph_table_length);
    }

    if (log_level >= APP_LOG_LEVEL_DEBUG) {
        uint16_t glyph_count = 0;
        FGlyphRange* index = ffont_glyph_index(font);
        for (uint16_t k = 0; k < font->glyph_index_length; ++k) {
            FGlyphRange* r = index + k;
            glyph_count += (r->end - r->begin);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "U+%04X-%04X", r->begin, r->end - 1);
        }
        APP_LOG(APP_LOG_LEVEL_DEBUG, "gc:%d", glyph_count);
    }

    if (log_level >= APP_LOG_LEVEL_DEBUG_VERBOSE) {
        FGlyph* glyphs = ffont_glyph_table(font);
        for (uint16_t k = 0; k < font->glyph_table_length; ++k) {
            FGlyph* g = glyphs + k;
            APP_LOG(APP_LOG_LEVEL_DEBUG, "glyph[%d] |%d| : %d + %d",
                k, g->horiz_adv_x, g->path_data_offset, g->path_data_length);
        }
    }
}

void ffont_destroy(FFont* font) {
    free(font);
}
