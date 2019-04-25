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

static inline struct run_header *find_run(struct chunk_header *chunk, uint32_t offset)
{
    assert(chunk->type == CHUNK_TYPE_SMALL);
    uint32_t runidx = offset / SHM_RUN_UNIT_SIZE;
    if(runidx >= SHM_CHUNK_RUN_SIZE || !bitmap_get(chunk->small.bitmap, runidx)) {
        return NULL;
    }
    return (struct run_header*)((char*)chunk + runidx * SHM_RUN_UNIT_SIZE);
}

static inline uint32_t malloc_run(struct run_header *run)
{
    uint32_t offset = run->free;
    assert(offset != RUN_LIST_NULL);
    assert(run->used < run->total);

    char *next = (char*)run + offset;
    run->free = *(uint32_t*)next;
    run->used++;
    return offset;
}

static inline void free_run(struct run_header *run, uint32_t offset)
{
    assert(run->used > 0);
    char *next = (char*)run + offset;
    *(uint32_t*)next = run->free;
    run->free = offset;
    run->used--;
}

static inline bool run_full(struct run_header *run)
{
    return run->used == run->total;
}

static inline bool run_empty(struct run_header *run)
{
    return run->used == 0;
}

extern void init_chunk(uint64_t pos, uint32_t type);
extern struct run_header *malloc_chunk_run(struct chunk_header *chunk, uint32_t type);
extern bool free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif

