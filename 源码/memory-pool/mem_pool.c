#include "mem_pool.h"

static void mcFixedPool_init(mcFixedPool *object);
static void* mcFixedPool_alloc(mcMemoryPool *pool, uint32_t size);
static void mcFixedPool_free(mcMemoryPool *pool, void *ptr);
static void mcFixedPool_destroy(mcMemoryPool *pool);

void mcFixedPool_debug_slice(mcFixedPool_slice *slice);


