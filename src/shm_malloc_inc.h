#ifndef _SHM_MALLOC_INC_H
#define _SHM_MALLOC_INC_H

#include <assert.h>
#include "shm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct shm_malloc_context local_context;

extern void *get_or_update_arena_addr(uint32_t index);

static inline void *get_arena_addr(uint32_t index)
{
    return local_context.arena_addrs[index];
}

static inline void update_arena_addr(uint32_t index, void *addr)
{
    local_context.arena_addrs[index] = addr;
}

/*
get or update arena addr from local_context, do not lock shared_context
if local_context not exist, mmap from shared_context
parameters:
    pos: find position
return addr, NULL for fail
*/
extern void *get_or_update_addr(uint64_t pos);

#ifdef __cplusplus
}
#endif

#endif
