#ifndef _SHM_CHUNK_H_
#define _SHM_CHUNK_H_

#include <assert.h>
#include "shm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RUN_LIST_NULL 0xffffffff

// run config info, start from 1
extern const struct run_config run_config_list[RUN_CONFIG_SIZE];
extern uint32_t find_run_config(size_t size);

static inline uint32_t run_idx2offset(uint32_t idx)
{
    return (idx + SHM_CHUNK_HEADER_HOLD) * SHM_RUN_UNIT_SIZE;
}

static inline uint32_t run_offset2idx(uint32_t offset)
{
    return offset / SHM_RUN_UNIT_SIZE - SHM_CHUNK_HEADER_HOLD;
}

static inline uint32_t malloc_run(struct chunk_run *run, void *addr)
{
    uint32_t offset = run->free;
    assert(offset != RUN_LIST_NULL);
    assert(run->used < run->total);

    char *next = (char*)addr + offset;
    run->free = *(uint32_t*)next;
    run->used++;
    return offset;
}

static inline void free_run(struct chunk_run *run, uint32_t offset, void *addr)
{
    char *next = (char*)addr + offset;
    *(uint32_t*)next = run->free;
    run->free = offset;
    run->used--;
}

static inline bool run_full(struct chunk_run *run)
{
    return run->used == run->total;
}

static inline bool run_empty(struct chunk_run *run)
{
    return run->used == 0;
}

extern void init_chunk(uint64_t pos, uint32_t type);
extern uint64_t malloc_chunk_small(struct chunk_header *chunk, uint32_t runtype);
extern void free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif

