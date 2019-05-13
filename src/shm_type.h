#ifndef _SHM_TYPE_H_
#define _SHM_TYPE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>
#include "shm_util.h"
#include "shm_tree.h"

#define SHM_INIT_FLAG 0x1020304a
#define SHM_PAGE_SIZE (4 * 1024)	// 4 KB

#define SHM_CHUNK_UNIT_SIZE	(1024 * 1024)	// 1 MB
#define SHM_RUN_UNIT_SIZE	SHM_PAGE_SIZE

#define SHM_ARENA_UNIT_SIZE (8 * 1024 * 1024)	// 8 MB
#define SHM_ARENA_CHUNK_SIZE (SHM_ARENA_UNIT_SIZE/ SHM_CHUNK_UNIT_SIZE)

#define SHM_SHARED_CONTEXT_SIZE SHM_CHUNK_UNIT_SIZE

static inline uint64_t index2pos(uint32_t index, uint32_t offset)
{
    return (uint64_t)index << 32 | (uint64_t)offset;
}

static inline uint32_t pos2index(uint64_t pos)
{
    return (uint32_t)(pos >> 32);
}

static inline uint32_t pos2offset(uint64_t pos)
{
    return (uint32_t)pos;
}

#define CHUNK_SMALL_LIMIT 1920
#define CHUNK_MEDIUM_LIMIT (SHM_CHUNK_UNIT_SIZE - SHM_PAGE_SIZE)
#define RUN_CONFIG_SIZE 15

#define CHUNK_TYPE_EMPTY 0
#define CHUNK_TYPE_SMALL 1
#define CHUNK_TYPE_MEDIUM 2

struct chunk_node_holder {
    char c[64];
};

struct chunk_detial_holder {
    char c[128];
};

struct chunk_header {
    struct chunk_node_holder node;	// must be first
    uint32_t type;
    uint64_t pos;
    struct chunk_detial_holder detial;
};

#define ARENA_TYPE_EMPTY 0
#define ARENA_TYPE_CONTEXT 1
#define ARENA_TYPE_CHUNK 2
#define ARENA_TYPE_LARGE 3

struct shm_arena {
    uint32_t type;
    int shmid;
    uint32_t index;
    size_t size;
    // follow feild not used when type == ARENA_TYPE_LARGE
    uint32_t used;
    uint64_t chunks[SHM_ARENA_CHUNK_SIZE];
};

static inline uint64_t get_arena_chunk(struct shm_arena *arena, uint32_t offset) {
    if(offset >= arena->size)
        return SHM_NULL;
    return arena->chunks[offset / SHM_CHUNK_UNIT_SIZE];
}

#define SHM_ARENA_MAX 128

struct context_chunk_holder {
    char c[128];
};

struct shm_shared_context {
    uint32_t init_flag;
    key_t key;
    unsigned count;
    uint64_t user_data;

    pthread_mutex_t mutex;	// locker for all shared memory allocator
    struct shm_arena arenas[SHM_ARENA_MAX]; // 0 used by self
    struct shm_pool run_pool[RUN_CONFIG_SIZE];
    struct context_chunk_holder chunk_small_pool;
    struct context_chunk_holder chunk_medium_pool;
};

struct shm_arena_addr {
    void *addr;
    int shmid;
};

struct shm_malloc_context {
    char *path;
    struct shm_shared_context *shared_context;
    struct shm_arena_addr arena_addrs[SHM_ARENA_MAX];
};

#endif
