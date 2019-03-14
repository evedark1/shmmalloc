#ifndef _SHM_TYPE_H
#define _SHM_TYPE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>
#include "shm_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHM_INIT_FLAG 0x1020304a

#define SHM_RUN_UNIT_SIZE	(4 * 1024)		// 4 KB
#define SHM_RUN_REG_SIZE 	(SHM_RUN_UNIT_SIZE / 8)

#define SHM_CHUNK_UNIT_SIZE	(1024 * 1024)	// 1 MB
#define SHM_CHUNK_HEADER_HOLD 5
#define SHM_CHUNK_RUN_SIZE  (SHM_CHUNK_UNIT_SIZE / SHM_RUN_UNIT_SIZE - SHM_CHUNK_HEADER_HOLD)

#define SHM_ARENA_UNIT_SIZE (8 * 1024 * 1024)	// 8 MB
#define SHM_ARENA_CHUNK_SIZE (SHM_ARENA_UNIT_SIZE/ SHM_CHUNK_UNIT_SIZE)

#define SHM_SHARED_CONTEXT_SIZE SHM_CHUNK_UNIT_SIZE

#define SHM_ARENA_MAX 256

union shm_pointer {
    uint64_t pos;
    struct {
        uint32_t index :32;
        uint32_t offset :32;
    } segment;
};

static inline uint64_t index2pos(uint32_t index, uint32_t offset)
{
    union shm_pointer pointer;
    pointer.segment.index = index;
    pointer.segment.offset = offset;
    return pointer.pos;
}

static inline uint32_t pos2index(uint64_t pos)
{
    union shm_pointer pointer;
    pointer.pos = pos;
    return pointer.segment.index;
}

static inline uint32_t pos2offset(uint64_t pos)
{
    union shm_pointer pointer;
    pointer.pos = pos;
    return pointer.segment.offset;
}

struct shm_list {
    uint64_t next;
    uint64_t prev;
};

struct chunk_run {
    uint32_t elemtype;
    bitmap_t bitmap[BITMAP_BITS2GROUPS(SHM_RUN_REG_SIZE)];
};

struct chunk_small {
    struct chunk_run runs[SHM_CHUNK_RUN_SIZE];
    bitmap_t bitmap[BITMAP_BITS2GROUPS(SHM_CHUNK_RUN_SIZE)];
};

#define CHUNK_TYPE_EMPTY 0
#define CHUNK_TYPE_SMALL 1
#define CHUNK_TYPE_MEDIUM 2

struct chunk_header {
    uint32_t type;
    uint64_t pos;
    struct chunk_small small;
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
    uint64_t chunks[SHM_ARENA_CHUNK_SIZE];
};

struct shm_shared_context {
    uint32_t init_flag;
    key_t key;
    unsigned count;
    uint64_t user_data;

    pthread_mutex_t mutex;	// locker for all shared memory allocator
    struct shm_arena arenas[SHM_ARENA_MAX]; // 0 used by self
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

#ifdef __cplusplus
}
#endif

#endif
