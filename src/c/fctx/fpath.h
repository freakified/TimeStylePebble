
#pragma once
#include "fctx.h"

typedef struct __attribute__((__packed__)) {
    uint16_t size;
    uint8_t data[];
} FPath;

FPath* fpath_load_from_resource_into_buffer(uint32_t resource_id, void* buffer);
FPath* fpath_create_from_resource(uint32_t resource_id);
void fpath_destroy(FPath* fpath);
