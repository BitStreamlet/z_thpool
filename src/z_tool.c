#include "z_tool.h"
#include "z_debug.h"
#include "z_kfifo.h"


struct z_tool_malloc_struct {
    unsigned int malloc_count;
    unsigned int realloc_count;
    unsigned int calloc_count;
    unsigned int strdup_count;
    unsigned int free_count;
};

static struct z_tool_malloc_struct gs_tool_malloc_struct = {0};

void *_z_tool_malloc(unsigned int size, const char *file, const char *function, unsigned int line, void *addr){
    void *p = NULL; 
    p = malloc(size);
    if (p == NULL) {
        Z_ERROR("malloc failed");
        return NULL;
    }
    gs_tool_malloc_struct.malloc_count++;
    return p;           
 }

 void *_z_tool_realloc(void *old_ptr, unsigned int size, const char *file, const char *function, unsigned int line, void *addr){
    void *p = NULL;
    p = realloc(old_ptr, size);
    if (p == NULL) {
        Z_ERROR("realloc failed");
        return NULL;
    }
    gs_tool_malloc_struct.realloc_count++;
    return p;
 }
 
 void _z_tool_free(void *ptr, const char *file, const char *function, unsigned int line){
    free(ptr);
    gs_tool_malloc_struct.free_count++;
 }

 void *_z_tool_calloc(unsigned int num, unsigned int size, const char *file, const char *function, unsigned int line, void *addr){
    void *p = NULL;
    p = calloc(num, size);
    if (p == NULL) {
        Z_ERROR("calloc failed");
        return NULL;
    }
    gs_tool_malloc_struct.calloc_count++;
    return p;
 }

 void *_z_tool_strdup(const char *s, const char *file, const char *function, unsigned int line, void *addr){
    void *p = NULL;
    p = strdup(s);
    if (p == NULL) {
        Z_ERROR("strdup failed");
        return NULL;
    }
    gs_tool_malloc_struct.strdup_count++;
    return p;
}

void z_tool_malloc_info(void){
    Z_INFO("malloc_count: %d", gs_tool_malloc_struct.malloc_count);
    Z_INFO("realloc_count: %d", gs_tool_malloc_struct.realloc_count);
    Z_INFO("calloc_count: %d", gs_tool_malloc_struct.calloc_count);
    Z_INFO("strdup_count: %d", gs_tool_malloc_struct.strdup_count);
    Z_INFO("free_count: %d", gs_tool_malloc_struct.free_count);
}   