#ifndef _CHUNK_SMALL_H_
#define _CHUNK_SMALL_H_

#include <assert.h>
#include "shm_type.h"

#define RUN_LIST_NULL 0xffffffff

struct run_header {
    struct shm_tree_node node;	// must be first

    uint64_t pos;
    uint32_t conf_index;
    uint32_t elemsize;
    uint32_t total;

    uint32_t used;
    uint32_t free;
};

struct run_config {
    uint32_t index;
    uint32_t elemsize;
};

// run config info, start from 1
extern const struct run_config run_config_list[RUN_CONFIG_SIZE];
extern uint32_t find_run_config(size_t size);
extern struct run_header *find_run(struct chunk_header *chunk, uint32_t offset);

extern void init_chunk_small(struct chunk_header *chunk);
extern struct run_header *new_chunk_run(struct chunk_header *chunk, uint32_t type);
extern uint64_t malloc_chunk_run(struct run_header *run);
extern bool free_chunk_small(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk_small(struct chunk_header *chunk, uint32_t offset);

#endif
