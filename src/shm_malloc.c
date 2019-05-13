#include "shm_malloc.h"
#include "shm_malloc_inc.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "shm_context.h"
#include "shm_util.h"

struct shm_malloc_context local_context;

static void shm_exit()
{
    if(local_context.shared_context != NULL)
        exit_shared_context(local_context.shared_context);
}

int shm_init(const char *path, int create)
{
    memset(&local_context, 0, sizeof(local_context));
    if((local_context.path = copystr(path)) == NULL)
        return -ENOMEM;
    if(atexit(shm_exit) != 0)
        return -1;

    struct shm_shared_context *context;
    if((context = init_shared_context(path, create)) == NULL)
        return -1;

    local_context.shared_context = context;
    lock_context();
    set_arena_addr(0, context, context->arenas[0].shmid);
    for(uint32_t i = 1; i < SHM_ARENA_MAX; i++) {
        if(is_arena_valid(context, i)) {
            set_arena_addr(i, mmap_arena(context, i), context->arenas[i].shmid);
        }
    }
    unlock_context();

    return 0;
}

uint64_t shm_malloc(size_t size)
{
    if(size == 0)
        return SHM_NULL;

    lock_context();
    uint64_t pos = malloc_arena(size);
    unlock_context();
    return pos;
}

void shm_free(uint64_t pos)
{
    if(pos == SHM_NULL)
        return;
    uint32_t index = pos2index(pos);
    uint32_t offset = pos2offset(pos);

    lock_context();
    free_arena(index, offset);
    unlock_context();
}

uint64_t shm_realloc(uint64_t pos, size_t size)
{
    if(pos == SHM_NULL)
        return shm_malloc(size);
    uint32_t index = pos2index(pos);
    uint32_t offset = pos2offset(pos);

    uint64_t ret = pos;
    lock_context();
    size_t checksize = check_arena(index, offset);
    if(size > checksize && checksize != 0) {
        ret = malloc_arena(size);
        if(ret != SHM_NULL) {
            void *src = get_or_update_addr(pos);
            void *dest = get_or_update_addr(ret);
            memcpy(dest, src, checksize);
            free_arena(index, offset);
        }
    }
    unlock_context();
    return ret;
}

size_t shm_check_size(uint64_t pos)
{
    if(pos == SHM_NULL)
        return 0;
    uint32_t index = pos2index(pos);
    uint32_t offset = pos2offset(pos);

    lock_context();
    size_t checksize = check_arena(index, offset);
    unlock_context();
    return checksize;
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
    uint32_t index = pos2index(pos);
    if(index >= SHM_ARENA_MAX)
        return NULL;

    lock_context();
    void *base = get_or_update_arena_addr(index);
    unlock_context();

    if(base == NULL)
        return NULL;
    return (char*)base + pos2offset(pos);
}

void *get_or_update_arena_addr(uint32_t index)
{
    struct shm_shared_context *context = local_context.shared_context;
    struct shm_arena_addr *arena = local_context.arena_addrs + index;
    if(arena->shmid != context->arenas[index].shmid) {
        if(arena->shmid != 0) {
            assert(arena->addr != NULL);
            munmap_arena(arena->addr);
            arena->addr = NULL;
            arena->shmid = 0;
        }
        if(is_arena_valid(context, index)) {
            arena->addr = mmap_arena(context, index);
            arena->shmid = context->arenas[index].shmid;
        }
    }
    return arena->addr;
}
