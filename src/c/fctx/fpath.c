
#include "fpath.h"

FPath* fpath_create_from_resource(uint32_t resource_id) {
    ResHandle res_handle = resource_get_handle(resource_id);
    size_t res_size = resource_size(res_handle);
    size_t path_size = res_size + sizeof(FPath);
    FPath* fpath = (FPath*)malloc(path_size);
    if (fpath) {
        fpath->size = (int16_t)res_size;
        resource_load(res_handle, fpath->data, res_size);
        return fpath;
    }
    return NULL;
}

FPath* fpath_load_from_resource_into_buffer(uint32_t resource_id, void* buffer) {
    ResHandle res_handle = resource_get_handle(resource_id);
    size_t res_size = resource_size(res_handle);
    FPath* fpath = (FPath*)buffer;
    if (fpath) {
        fpath->size = (int16_t)res_size;
        resource_load(res_handle, fpath->data, res_size);
        return fpath;
    }
    return NULL;
}

void fpath_destroy(FPath* fpath) {
    free(fpath);
}
