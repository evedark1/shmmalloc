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
    context->arenas[0].type = ARENA_TYPE_CONTEXT;
    context->arenas[0].shmid = shmid;
    context->arenas[0].index = 0;
    context->arenas[0].size = SHM_ARENA_UNIT_SIZE;
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
    struct shm_shared_context *context = shm_mmap(key, SHM_ARENA_UNIT_SIZE, create, &shmid, &err);
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

void munmap_arena(void *addr)
{
    shm_munmap(addr);
}

static struct shm_arena *new_arena(struct shm_shared_context *context, uint32_t type, size_t size)
{
    assert(size % SHM_CHUNK_UNIT_SIZE == 0);
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
    set_arena_addr(index, addr, shmid);
    logInfo("new arena create, index %u key %x shmid %u size %zu", index, key, shmid, size);
    return arena;
}

static void delete_arena(struct shm_shared_context *context, uint32_t index)
{
    struct shm_arena *arena = context->arenas + index;
    void *addr = get_arena_addr(index);
    assert(addr != NULL);

    memset(arena, 0, sizeof(struct shm_arena));
    clear_arena_addr(index);
    shm_munmap(addr);
}

static uint64_t new_arena_chunk(struct shm_arena *arena)
{
    uint64_t ret = SHM_NULL;
    uint32_t idx = (arena->type == ARENA_TYPE_CONTEXT) ? 1 : 0;
    while(idx < SHM_ARENA_CHUNK_SIZE) {
        if(arena->chunks[idx] == 0) {
            ret = index2pos(arena->index, idx * SHM_CHUNK_UNIT_SIZE);
            arena->chunks[idx] = ret;
            break;
        }
        idx++;
    }
    return ret;
}

static uint64_t new_chunk(struct shm_shared_context *context, uint32_t type)
{
    uint64_t ret = SHM_NULL;
    uint32_t emptyIndex = 0;
    // find arena can alloc chunk
    for(uint32_t i = 0; i < SHM_ARENA_MAX; i++) {
        struct shm_arena *arena = context->arenas + i;
        if(arena->type == ARENA_TYPE_CONTEXT || arena->type == ARENA_TYPE_CONTEXT) {
            if((ret = new_arena_chunk(arena)) != SHM_NULL)
                break;
        } else if(arena->type == ARENA_TYPE_EMPTY) {
            emptyIndex = i;
        }
    }

    // alloc new arena
    if(ret == SHM_NULL && emptyIndex != 0) {
        struct shm_arena *arena = new_arena(context, ARENA_TYPE_CHUNK, SHM_ARENA_UNIT_SIZE);
        if(arena != NULL) {
            ret = new_arena_chunk(arena);
            assert(ret != SHM_NULL);
        }
    }

    // init new chunk
    if(ret != SHM_NULL) {
        init_chunk(ret, type);
        if(type == CHUNK_TYPE_SMALL) {
            shm_tree_push(&context->chunk_small_pool, ret);
        } else {
            shm_tree_push(&context->chunk_medium_pool, ret);
        }
    }
    return ret;
}

static void delete_chunk(struct shm_shared_context *context, struct chunk_header *chunk)
{
    if(chunk->type == CHUNK_TYPE_SMALL) {
        shm_tree_remove(&context->chunk_small_pool, chunk->pos);
    } else {
        shm_tree_remove(&context->chunk_medium_pool, chunk->pos);
    }
    struct shm_arena *arena = context->arenas + pos2index(chunk->pos);
    uint32_t idx = pos2offset(chunk->pos) / SHM_CHUNK_UNIT_SIZE;
    assert(arena->chunks[idx] != 0);
    arena->chunks[idx] = 0;
    //TODO: delete arena
}

static uint64_t get_or_new_chunk(struct shm_shared_context *context, uint32_t type)
{
    uint64_t chunk_pos = SHM_NULL;
    switch(type) {
    case CHUNK_TYPE_SMALL:
        chunk_pos = context->chunk_small_pool.root;
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: medium type
        break;
    }
    if(chunk_pos == SHM_NULL)
        chunk_pos = new_chunk(context, type);
    return chunk_pos;
}

static void check_delete_chunk(struct shm_shared_context *context, struct chunk_header *chunk)
{
    //TODO: delete chunk
    /*
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        if(bitmap_ffu(chunk->c.small.bitmap, SHM_CHUNK_RUN_SIZE) == SHM_CHUNK_RUN_SIZE) {
            delete_chunk(context, chunk);
        }
        break;
    case CHUNK_TYPE_MEDIUM:
        break;
    }
    */
}

static struct run_header *try_run_pool(struct shm_shared_context *context, uint32_t runidx)
{
    struct shm_pool *pool = context->run_pool + runidx;
    uint64_t run_pos = pool->root;
    if(run_pos == SHM_NULL)
        return NULL;

    struct run_header *run = get_or_update_addr(run_pos);
    assert(run != NULL);
    return run;
}

uint64_t malloc_arena(size_t size)
{
    struct shm_shared_context *context = local_context.shared_context;
    uint64_t ret = SHM_NULL;
    if(size <= CHUNK_SMALL_LIMIT) {
        uint32_t runidx = find_run_config(size);
        struct run_header *run = try_run_pool(context, runidx);

        if(run == NULL) {
            uint64_t chunk_pos = get_or_new_chunk(context, CHUNK_TYPE_SMALL);
            if(chunk_pos == SHM_NULL) {
                logNotice("malloc arena new chunk full");
                return SHM_NULL;
            }
            run = malloc_chunk_run(get_or_update_addr(chunk_pos), runidx);
        }
        assert(run != NULL);
        ret = run->pos + malloc_run(run);
        if(run_full(run)) {
            shm_tree_pop(context->run_pool + runidx);
        }
    } else if(size <= CHUNK_MEDIUM_LIMIT) {
        uint64_t chunk_pos = get_or_new_chunk(context, CHUNK_TYPE_MEDIUM);
        if(chunk_pos == SHM_NULL) {
            logNotice("malloc arena new chunk full");
            return SHM_NULL;
        }
        ret = malloc_chunk_medium(get_or_update_addr(chunk_pos), size);
    } else {
        size_t asize = align_size(size, SHM_CHUNK_UNIT_SIZE);
        struct shm_arena *arena = new_arena(context, ARENA_TYPE_LARGE, asize);
        if(arena) {
            ret = index2pos(arena->index, 0);
        }
    }
    return ret;
}

void free_arena(uint32_t index, uint32_t offset)
{
    struct shm_shared_context *context = local_context.shared_context;
    if(!is_arena_valid(context, index)) {
        logNotice("free arena index error, %u %u", index, offset);
        return;
    }

    struct shm_arena *arena = context->arenas + index;
    uint64_t chunk_pos = SHM_NULL;

    switch(arena->type) {
    case ARENA_TYPE_CONTEXT:
    case ARENA_TYPE_CHUNK:
        chunk_pos = get_arena_chunk(arena, offset);
        if(chunk_pos != SHM_NULL) {
            struct chunk_header *chunk_header = get_or_update_addr(chunk_pos);
            uint32_t chunk_offset = offset - pos2offset(chunk_pos);
            assert(chunk_offset < SHM_CHUNK_UNIT_SIZE);

            bool check = free_chunk(chunk_header, chunk_offset);
            if(check) {
                check_delete_chunk(context, chunk_header);
            }
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

size_t check_arena(uint32_t index, uint32_t offset)
{
    struct shm_shared_context *context = local_context.shared_context;
    if(!is_arena_valid(context, index)) {
        logNotice("check arena index error, %u %u", index, offset);
        return 0;
    }
    size_t ret = 0;

    struct shm_arena *arena = context->arenas + index;
    uint64_t chunk_pos = SHM_NULL;

    switch(arena->type) {
    case ARENA_TYPE_CONTEXT:
    case ARENA_TYPE_CHUNK:
        chunk_pos = get_arena_chunk(arena, offset);
        if(chunk_pos != SHM_NULL) {
            struct chunk_header *chunk_header = get_or_update_addr(chunk_pos);
            uint32_t chunk_offset = offset - pos2offset(chunk_pos);
            assert(chunk_offset < SHM_CHUNK_UNIT_SIZE);

            ret = check_chunk(chunk_header, offset);
        } else {
            logNotice("check arena offset error, %u %u", index, offset);
        }
        break;
    case ARENA_TYPE_LARGE:
        ret = arena->size;
        break;
    default:
        assert(0);
        break;
    }
    return ret;
}

void lock_context()
{
    int err = pthread_mutex_lock(&local_context.shared_context->mutex);
    if(err != 0) {
        logError("lock shared context error %d", err);
    }
}

void unlock_context()
{
    int err = pthread_mutex_unlock(&local_context.shared_context->mutex);
    if(err != 0) {
        logError("unlock shared context error %d", err);
    }
}
