#include "shm_chunk.h"
#include <string.h>
#include "shm_logger.h"
#include "shm_util.h"
#include "shm_malloc_inc.h"

#define RUN_HEADER_SIZE 64
static_assert(sizeof(struct chunk_header) <= SHM_RUN_UNIT_SIZE, "chunk header too big");
static_assert(sizeof(struct run_header) <= SHM_RUN_UNIT_SIZE, "run header too big");

#define RUN_CONFIG_ITEM(index, elem) {index, elem}
const struct run_config run_config_list[RUN_CONFIG_SIZE] =
{
    {0, 0},
    // align 8
    {1, 8},
    {2, 16},
    {3, 32},
    {4, 48},
    {5, 64},
    {6, 96},
    // align 32
    {7, 128},
    {8, 192},
    {9, 256},
    {10, 384},
    {11, 512},
    // align 128
    {12, 768},
    {13, 1280},
    {14, CHUNK_SMALL_LIMIT},
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
    13, 13, 14, 14, 14, 14, 14, 14,
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

void init_chunk(uint64_t pos, uint32_t type)
{
    struct chunk_header *chunk = get_or_update_addr(pos);
    memset(chunk, 0, sizeof(struct chunk_header));
    chunk->pos = pos;
    chunk->type = type;
    if(type == CHUNK_TYPE_SMALL) {
        // first unit used by chunk_header
        bitmap_set(chunk->small.bitmap, 0);
    }
}

struct run_header *malloc_chunk_run(struct chunk_header *chunk, uint32_t type)
{
    assert(chunk->type == CHUNK_TYPE_SMALL);
    int32_t idx = bitmap_sfu(chunk->small.bitmap, SHM_CHUNK_RUN_SIZE);
    assert(idx < SHM_CHUNK_RUN_SIZE);

    // init chunk_run
    const struct run_config *conf = run_config_list + type;
    uint32_t offset = idx * SHM_RUN_UNIT_SIZE;
    struct run_header *run = (struct run_header*)((char*)chunk + offset);
    memset(run, 0, SHM_RUN_UNIT_SIZE);

    run->pos = chunk->pos + offset;
    run->conf_index = type;
    run->elemsize = conf->elemsize;
    run->total = (SHM_RUN_UNIT_SIZE - RUN_HEADER_SIZE) / run->elemsize;
    run->used = 0;
    run->free = RUN_HEADER_SIZE;

    // init run free list
    char *runblock = (char*)run + RUN_HEADER_SIZE;
    for(uint32_t i = 1; i < run->total; i++) {
        uint32_t diff = i * run->elemsize + RUN_HEADER_SIZE;
        *(uint32_t*)runblock = diff;
        runblock = runblock + run->elemsize;
    }
    *(uint32_t*)runblock = RUN_LIST_NULL;

    // add to run pool
    struct shm_shared_context *context = local_context.shared_context;
    shm_tree_push(context->run_pool + type, run->pos);
    return run;
}

void free_chunk_run(struct chunk_header *chunk, uint64_t pos)
{
    uint32_t idx = (pos - chunk->pos) / SHM_RUN_UNIT_SIZE;
    assert(bitmap_get(chunk->small.bitmap, idx));
    bitmap_clear(chunk->small.bitmap, idx);
}

static bool free_chunk_small(struct chunk_header *chunk, uint32_t offset)
{
    bool check = false;
    struct run_header *run = find_run(chunk, offset);
    if(run == NULL) {
        logNotice("free chunk small offset out of range");
        return check;
    }

    bool full = run_full(run);
    uint32_t run_offset = (uint64_t)(chunk->pos + offset) - run->pos;
    free_run(run, run_offset);

    if(full) {
        struct shm_shared_context *context = local_context.shared_context;
        shm_tree_push(context->run_pool + run->conf_index , run->pos);
    }
    if(run_empty(run)) {
        struct shm_shared_context *context = local_context.shared_context;
        struct shm_pool *pool = context->run_pool + run->conf_index;
        if(pool->size > 1) {
            shm_tree_remove(pool, run->pos);
            free_chunk_run(chunk, run->pos);
            check = true;
        }
    }
    return check;
}

bool free_chunk(struct chunk_header *chunk, uint32_t offset)
{
    bool check = false;
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        check = free_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: chunk medium
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
    return check;
}

static size_t check_chunk_small(struct chunk_header *chunk, uint32_t offset)
{
    struct run_header *run = find_run(chunk, offset);
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
