#ifndef _SHM_MALLOC_INC_H
#define _SHM_MALLOC_INC_H

#include <assert.h>
#include "shm_malloc.h"
#include "shm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct shm_malloc_context local_context;

static inline void *get_arena_addr(uint32_t index)
{
    return local_context.arena_addrs[index].addr;
}

static inline void set_arena_addr(uint32_t index, void *addr, int shmid)
{
    local_context.arena_addrs[index].addr = addr;
    local_context.arena_addrs[index].shmid= shmid;
}

static inline void clear_arena_addr(uint32_t index)
{
    local_context.arena_addrs[index].addr = NULL;
    local_context.arena_addrs[index].shmid= 0;
}

extern void *get_or_update_arena_addr(uint32_t index);

/*
get or update arena addr from local_context, do not lock shared_context
if local_context not exist, mmap from shared_context
parameters:
    pos: find position
return addr, NULL for fail
*/
static inline void *get_or_update_addr(uint64_t pos)
{
    if(pos == SHM_NULL)
        return NULL;
    uint32_t index = pos2index(pos);
    void *base = get_or_update_arena_addr(index);
    if(base == NULL)
        return NULL;
    return (char*)base + pos2offset(pos);
}

#ifdef __cplusplus
}
#endif

#endif
