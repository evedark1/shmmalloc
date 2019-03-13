#include "shm_context.h"
#include <assert.h>
#include "shm_logger.h"
#include "shm_malloc.h"
#include "shm_malloc_inc.h"
#include "shm_chunk.h"
#include "shm_lock.h"
#include "shm_op_wrapper.h"

static_assert(sizeof(struct shm_shared_context) < SHM_SHARED_CONTEXT_SIZE, "shared context too big");

// init shm_shared_context field
static void init_shm_shared_field(struct shm_shared_context *context, key_t key)
{
    memset(context, 0, sizeof(struct shm_shared_context));
    context->init_flag = SHM_INIT_FLAG;
    shm_lock_init(&context->mutex);
    context->arenas[0].key = key;
    context->arenas[0].size = SHM_SHARED_CONTEXT_SIZE;
}

struct shm_shared_context *init_shared_context(const char *path, bool create)
{
    int err;
    key_t key;
    struct shm_shared_context *context = shm_mmap(path, 0, SHM_SHARED_CONTEXT_SIZE,
            &key, create, &err);
    if(context == NULL) {
        logError("init shared context path %s error %d", path, err);
        return NULL;
    }
    if(create) {
        int lockfd = shm_lock_file(path);
        if(lockfd < 0) {
            logError("init shared context path %s lock error", path);
            return NULL;
        }
        if(context->init_flag != SHM_INIT_FLAG) {
            init_shm_shared_field(context, key);
        }
        shm_unlock_file(lockfd);
    }
    logInfo("init shared context path %s key %d", path, key);
    return context;
}

static inline uint64_t get_arena_chunk(struct shm_arena *arena, uint32_t offset) {
    if(offset >= arena->size)
        return SHM_NULL;
    return arena->chunks[offset / SHM_CHUNK_UNIT_SIZE];
}

static struct shm_arena *new_arena(struct shm_shared_context *context, uint32_t type, size_t size)
{
    assert(size / SHM_CHUNK_UNIT_SIZE == 0);
    uint32_t index;
    for(index = 1; index < SHM_ARENA_MAX; index++) {
        if(context->arenas[index].type == ARENA_TYPE_EMPTY)
            break;
    }
    if(index == SHM_ARENA_MAX) {
        logNotice("new arena size max");
        return NULL;
    }

    key_t key = 0;
    int err = 0;
    void *addr = shm_mmap(local_context.path, index, size, &key, true, &err);
    if(addr == NULL) {
        logError("shm mmap fail, index %u size %zu", index, size);
        return NULL;
    }
    memset(addr, 0, size);

    struct shm_arena *arena = context->arenas + index;
    arena->type = type;
    arena->key = key;
    arena->index = index;
    arena->size = size;
    update_arena_addr(index, addr);
    logInfo("new arena create, index %u key %u size %zu", index, key, size);
    return arena;
}

static void delete_arena(struct shm_shared_context *context, uint32_t index)
{
    struct shm_arena *arena = context->arenas + index;
    size_t size = arena->size;
    void *addr = get_arena_addr(index);
    assert(addr != NULL);

    memset(arena, 0, sizeof(struct shm_arena));
    update_arena_addr(index, NULL);
    shm_munmap(addr, size);
}

uint64_t malloc_arena(struct shm_shared_context *context, size_t size)
{
    uint64_t ret = 0;
    if(size <= CHUNK_SMALL_LIMIT) {
        //TODO: small alloc
    } else if(size <= CHUNK_MEDIA_LIMIT) {
        //TODO: media alloc
    } else {
        size_t asize = align_size(size, SHM_CHUNK_UNIT_SIZE);
        struct shm_arena *arena = new_arena(context, ARENA_TYPE_LARGE, asize);
        if(arena) {
            ret = index2pos(arena->index, 0);
        }
    }
    return ret;
}

void free_arena(struct shm_shared_context *context, uint32_t index, uint32_t offset)
{
    if(!is_arena_valid(context, index)) {
        logNotice("free arena index error, %u %u", index, offset);
        return;
    }

    struct shm_arena *arena = context->arenas + index;
    uint64_t chunk_pos = SHM_NULL;
    struct chunk_header *chunk_header = NULL;

    switch(arena->type) {
    case ARENA_TYPE_CONTEXT:
    case ARENA_TYPE_CHUNK:
        chunk_pos = get_arena_chunk(arena, offset);
        if(chunk_pos != SHM_NULL) {
            chunk_header = get_or_update_addr(chunk_pos);
            free_chunk(chunk_header, offset);
        } else {
            logNotice("free arena offset error, %u %u", index, offset);
        }
        break;
    case ARENA_TYPE_LARGE:
        delete_arena(context, index);
        break;
    default:
        assert(0);
        break;
    }
}

void lock_context(struct shm_shared_context *context)
{
    int err = pthread_mutex_lock(&context->mutex);
    if(err != 0) {
        logError("lock shared context error %d", err);
    }
}

void unlock_context(struct shm_shared_context *context)
{
    int err = pthread_mutex_unlock(&context->mutex);
    if(err != 0) {
        logError("unlock shared context error %d", err);
    }
}
