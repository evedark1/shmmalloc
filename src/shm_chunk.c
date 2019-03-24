#include "shm_chunk.h"
#include <string.h>
#include "shm_logger.h"
#include "shm_util.h"
#include "shm_malloc_inc.h"

static_assert(sizeof(struct chunk_header) <= SHM_RUN_UNIT_SIZE * SHM_CHUNK_HEADER_HOLD, "chunk header too big");

#define RUN_CONFIG_ITEM(index, elem) {index, elem, SHM_RUN_UNIT_SIZE / elem}
const struct run_config run_config_list[RUN_CONFIG_SIZE] =
{
    {0, 0, 0},
    // align 8
    RUN_CONFIG_ITEM(1, 8),
    RUN_CONFIG_ITEM(2, 16),
    RUN_CONFIG_ITEM(3, 32),
    RUN_CONFIG_ITEM(4, 48),
    RUN_CONFIG_ITEM(5, 64),
    RUN_CONFIG_ITEM(6, 96),
    // align 32
    RUN_CONFIG_ITEM(7, 128),
    RUN_CONFIG_ITEM(8, 192),
    RUN_CONFIG_ITEM(9, 256),
    RUN_CONFIG_ITEM(10, 384),
    RUN_CONFIG_ITEM(11, 512),
    // align 128
    RUN_CONFIG_ITEM(12, 768),
    RUN_CONFIG_ITEM(13, 1024),
    RUN_CONFIG_ITEM(14, 2048),
};

// align 8
const static uint32_t run_table8[12] =
{
    1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6
};

// align 32
const static uint32_t run_table32[16] =
{
    7, 7, 7, 7, 8, 8, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11
};

// align 128
const static uint32_t run_table128[16] =
{
    12, 12, 12, 12, 12, 12, 13, 13,
    14, 14, 14, 14, 14, 14, 14, 14,
};

uint32_t find_run_config(size_t size)
{
    assert(size > 0 && size <= CHUNK_SMALL_LIMIT);
    uint32_t ret = 0;
    if(size <= 96) {
        uint32_t idx = (size - 1) / 8;
        ret = run_table8[idx];
    } else if(size <= 512) {
        uint32_t idx = (size - 1) / 32;
        ret = run_table32[idx];
    } else {
        uint32_t idx = (size - 1) / 128;
        ret = run_table128[idx];
    }
    return ret;
}

static inline struct chunk_run *find_run(struct chunk_header *chunk, uint32_t offset)
{
    assert(chunk->type == CHUNK_TYPE_SMALL);
    uint32_t chunk_offset = offset - pos2offset(chunk->pos);
    uint32_t runidx = chunk_offset / SHM_RUN_UNIT_SIZE - SHM_CHUNK_HEADER_HOLD;
    if(runidx >= SHM_CHUNK_RUN_SIZE || !bitmap_get(chunk->small.bitmap, runidx)) {
        return NULL;
    }
    return chunk->small.runs + runidx;
}

static void free_chunk_small(struct chunk_header *chunk, uint32_t offset)
{
    uint32_t chunk_offset = offset - pos2offset(chunk->pos);
    uint32_t runidx = chunk_offset / SHM_RUN_UNIT_SIZE - SHM_CHUNK_HEADER_HOLD;
    if(runidx >= SHM_CHUNK_RUN_SIZE || !bitmap_get(chunk->small.bitmap, runidx)) {
        logNotice("free chunk small offset out of range");
        return;
    }

    struct chunk_run *run = chunk->small.runs + runidx;
    uint32_t run_offset = offset % SHM_RUN_UNIT_SIZE;
    if(run_offset % run->elemsize != 0) {
        logNotice("free chunk small offset not valid");
        return;
    }
    void *addr = (char*)chunk + (offset - run_offset);
    free_run(run, run_offset, addr);
}

void init_chunk(uint64_t pos, uint32_t type)
{
    struct chunk_header *chunk = get_or_update_addr(pos);
    memset(chunk, 0, sizeof(struct chunk_header));
    chunk->pos = pos;
    chunk->type = type;
}

uint64_t malloc_chunk_small(struct chunk_header *chunk, uint32_t type)
{
    assert(chunk->type == CHUNK_TYPE_SMALL);
    int32_t idx = bitmap_sfu(chunk->small.bitmap, SHM_CHUNK_RUN_SIZE);
    assert(idx < SHM_CHUNK_RUN_SIZE);

    // init chunk_run
    uint32_t offset = run_idx2offset(idx);
    const struct run_config *conf = run_config_list + type;
    struct chunk_run *run = chunk->small.runs + idx;
    run->pos = chunk->pos + offset;
    run->conf_index = type;
    run->elemsize = conf->elemsize;
    run->total = SHM_RUN_UNIT_SIZE / run->elemsize;
    run->used = 0;
    run->free = 0;
    // init run block
    char *runblock = (char*)get_or_update_addr(chunk->pos) + offset;
    for(uint32_t i = 0; i < run->total - 1; i++) {
        uint32_t diff = i * run->elemsize;
        char *addr = runblock + diff;
        *(uint32_t*)addr = diff + run->elemsize;
    }
    char *endptr = runblock + run->elemsize * (run->total - 1);
    *(uint32_t*)endptr = RUN_LIST_NULL;

    return run->pos + malloc_run(run, runblock);
}

void free_chunk(struct chunk_header *chunk, uint32_t offset)
{
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        free_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: chunk medium
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
}

static size_t check_chunk_small(struct chunk_header *chunk, uint32_t offset)
{
    struct chunk_run *run = find_run(chunk, offset);
    return run->elemsize;
}

size_t check_chunk(struct chunk_header *chunk, uint32_t offset)
{
    size_t ret = 0;
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        ret = check_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: chunk medium
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
    return ret;
}
