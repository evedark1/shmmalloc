#include "shm_chunk.h"
#include <assert.h>
#include "shm_logger.h"
#include "shm_util.h"

static_assert(sizeof(struct chunk_header) <= SHM_RUN_UNIT_SIZE * SHM_CHUNK_HEADER_HOLD, "chunk header too big");

#define RUN_CONFIG_ITEM(elem) {elem, SHM_RUN_UNIT_SIZE / elem}
const static struct run_config run_config_list[RUN_CONFIG_SIZE] =
{
    {0, 0},
    // align 8
    RUN_CONFIG_ITEM(8),
    RUN_CONFIG_ITEM(16),
    RUN_CONFIG_ITEM(32),
    RUN_CONFIG_ITEM(48),
    RUN_CONFIG_ITEM(64),
    RUN_CONFIG_ITEM(96),
    // align 32
    RUN_CONFIG_ITEM(128),
    RUN_CONFIG_ITEM(192),
    RUN_CONFIG_ITEM(256),
    RUN_CONFIG_ITEM(384),
    RUN_CONFIG_ITEM(512),
    // align 128
    RUN_CONFIG_ITEM(768),
    RUN_CONFIG_ITEM(1024),
    RUN_CONFIG_ITEM(2048),
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

extern const struct run_config *find_run_config(size_t size)
{
    assert(size > 0 && size <= CHUNK_SMALL_LIMIT);
    const struct run_config *conf = NULL;
    if(size <= 96) {
        uint32_t idx = (size - 1) / 8;
        conf = run_config_list + run_table8[idx];
    } else if(size <= 512) {
        uint32_t idx = (size - 1) / 32;
        conf = run_config_list + run_table32[idx];
    } else {
        uint32_t idx = (size - 1) / 128;
        conf = run_config_list + run_table128[idx];
    }
    return conf;
}

static inline uint32_t malloc_run(struct chunk_run *run)
{
    assert(run->elemtype != 0);
    const struct run_config *conf = run_config_list + run->elemtype;
    uint32_t bit = bitmap_sfu(run->bitmap, conf->regsize);
    assert(bit < conf->regsize);
    return bit * conf->elemsize;
}

static inline void free_run(struct chunk_run *run, uint32_t offset)
{
    assert(run->elemtype != 0);
    const struct run_config *conf = run_config_list + run->elemtype;
    uint32_t bit = offset / conf->elemsize;
    if(bit >= conf->regsize) {
        logNotice("free run offset error");
        return;
    }
    bitmap_clear(run->bitmap, bit);
}

static void free_chunk_small(struct chunk_header *chunk, uint32_t offset)
{
    union shm_pointer self;
    self.pos = chunk->pos;
    uint32_t chunk_offset = offset - self.segment.offset;
    uint32_t runidx = chunk_offset / SHM_RUN_UNIT_SIZE - SHM_CHUNK_HEADER_HOLD;
    if(runidx >= SHM_CHUNK_RUN_SIZE || !bitmap_get(chunk->small.bitmap, runidx)) {
        logNotice("free chunk small offset error");
        return;
    }

    free_run(chunk->small.runs + runidx, chunk_offset % SHM_RUN_UNIT_SIZE);
}

void free_chunk(struct chunk_header *chunk, uint32_t offset)
{
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        free_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: chunk media
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
}
