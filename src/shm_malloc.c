#include "shm_malloc.h"
#include "shm_malloc_inc.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "shm_context.h"
#include "shm_util.h"
#include "shm_op_wrapper.h"

struct shm_malloc_context local_context;

static inline void *mmap_arena(struct shm_shared_context *context, uint32_t index)
{
    struct shm_arena *arena = context->arenas + index;
    assert(arena->type != ARENA_TYPE_EMPTY && arena->type != ARENA_TYPE_CONTEXT);
    int err = 0;
    return shm_do_shmmap(arena->key, arena->size, false, &err);
}

int shm_init(const char *path, int create)
{
    memset(&local_context, 0, sizeof(local_context));

    if((local_context.shared_context = init_shared_context(path, create)) == NULL)
        return -1;
    local_context.arena_addrs[0] = local_context.shared_context;

    lock_context(local_context.shared_context);
    for(uint32_t i = 1; i < SHM_ARENA_MAX; i++) {
        if(is_arena_valid(local_context.shared_context, i))
            local_context.arena_addrs[i] = mmap_arena(local_context.shared_context, i);
    }
    unlock_context(local_context.shared_context);

    if((local_context.path = copystr(path)) == NULL)
        return -ENOMEM;

    return 0;
}

uint64_t shm_malloc(size_t size)
{
    if(size == 0)
        return SHM_NULL;

    lock_context(local_context.shared_context);
    uint64_t pos = malloc_arena(local_context.shared_context, size);
    unlock_context(local_context.shared_context);
    return pos;
}

void shm_free(uint64_t pos)
{
    if(pos == SHM_NULL)
        return;
    union shm_pointer pointer;
    pointer.pos = pos;

    lock_context(local_context.shared_context);
    free_arena(local_context.shared_context, pointer.segment.index, pointer.segment.offset);
    unlock_context(local_context.shared_context);
}

uint64_t shm_realloc(uint64_t pos, size_t size)
{
    return SHM_NULL;
}

void shm_set_userdata(uint64_t data)
{
    local_context.shared_context->user_data = data;
}

uint64_t shm_get_userdata()
{
    return local_context.shared_context->user_data;
}

void *shm_get_addr(uint64_t pos)
{
    union shm_pointer pointer;
    pointer.pos = pos;
    uint32_t index = pointer.segment.index;
    if(index >= SHM_ARENA_MAX)
        return NULL;

    void *base = local_context.arena_addrs[index];
    if(base == NULL) {
        lock_context(local_context.shared_context);

        if(is_arena_valid(local_context.shared_context, index)) {
            base = mmap_arena(local_context.shared_context, index);
            local_context.arena_addrs[index] = base;
        }

        unlock_context(local_context.shared_context);
    }
    if(base == NULL)
        return NULL;
    return (char*)base + pointer.segment.offset;
}

void *get_or_update_arena_addr(uint32_t index)
{
    void *base = local_context.arena_addrs[index];
    if(base == NULL) {
        if(is_arena_valid(local_context.shared_context, index)) {
            base = mmap_arena(local_context.shared_context, index);
            local_context.arena_addrs[index] = base;
        }
    }
    return base;
}

void *get_or_update_addr(uint64_t pos)
{
    union shm_pointer pointer;
    pointer.pos = pos;
    void *base = get_or_update_arena_addr(pointer.segment.index);
    if(base == NULL)
        return NULL;
    return (char*)base + pointer.segment.offset;
}
