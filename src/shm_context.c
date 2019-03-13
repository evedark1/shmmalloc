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
static void init_shm_shared_field(struct shm_shared_context *context, key_t key, int shmid)
{
    memset(context, 0, sizeof(struct shm_shared_context));
    context->init_flag = SHM_INIT_FLAG;
    context->key = key;
    context->count = 0;
    shm_lock_init(&context->mutex);
    context->arenas[0].shmid = shmid;
    context->arenas[0].size = SHM_SHARED_CONTEXT_SIZE;
}

struct shm_shared_context *init_shared_context(const char *path, bool create)
{
    int err;
    key_t key;
    int shmid = 0;
    if((err = shm_get_key(path, 0, &key)) != 0) {
        logError("init shared context get shm key fail, path %s error %d", path, err);
        return NULL;
    }
    struct shm_shared_context *context = shm_mmap(key, SHM_SHARED_CONTEXT_SIZE, create, &shmid, &err);
    if(context == NULL) {
        logError("init shared context path %s error %d", path, err);
        return NULL;
    }

    unsigned count = 1;
    if(create) {
        int lockfd = shm_lock_file(path);
        if(lockfd < 0) {
            logError("init shared context lock fail, path %s", path);
            return NULL;
        }
        if(context->init_flag != SHM_INIT_FLAG) {
            init_shm_shared_field(context, key, shmid);
        }
        shm_unlock_file(lockfd);
    }
    assert(SHM_INIT_FLAG == context->init_flag);
    assert(key == context->key);
    count = atomic_add_and_fetch(&context->count, 1);

    logInfo("init shared context path %s key %d count %u", path, key, count);
    return context;
}

void exit_shared_context(struct shm_shared_context *context)
{
    unsigned count = atomic_sub_and_fetch(&context->count, 1);
    if(count == 0) {
        shm_remove(context->arenas[0].shmid);
    }
}

static inline uint64_t get_arena_chunk(struct shm_arena *arena, uint32_t offset) {
    if(offset >= arena->size)
        return SHM_NULL;
    return arena->chunks[offset / SHM_CHUNK_UNIT_SIZE];
}

void *mmap_arena(struct shm_shared_context *context, uint32_t index)
{
    struct shm_arena *arena = context->arenas + index;
    assert(arena->type != ARENA_TYPE_EMPTY && arena->type != ARENA_TYPE_CONTEXT);
    int err = 0;
    int shmid = arena->shmid;
    return shm_mmap(0, arena->size, false, &shmid, &err);
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

    const char *path = local_context.path;
    key_t key = 0;
    int err = 0;
    int shmid = 0;
    if((err = shm_get_key(path, index, &key)) != 0) {
        logError("new arena get shm key fail, index %u path %s error %d", index, path, err);
        return NULL;
    }
    void *addr = shm_mmap(key, size, true, &shmid, &err);
    if(addr == NULL) {
        logError("new arena mmap fail, index %u key %x size %zu", index, key, size);
        return NULL;
    }
    // set SHM_DEST flag, segment will be destoryed after the last process detach
    if(shm_remove(shmid) != 0) {
        logError("new arena remove fail, shmid %u", shmid);
        return NULL;
    }
    memset(addr, 0, size);

    struct shm_arena *arena = context->arenas + index;
    arena->type = type;
    arena->shmid = shmid;
    arena->index = index;
    arena->size = size;
    update_arena_addr(index, addr);
    logInfo("new arena create, index %u key %x shmid %u size %zu", index, key, shmid, size);
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
