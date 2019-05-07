#ifndef _SHM_CONTEXT_H_
#define _SHM_CONTEXT_H_

#include "shm_type.h"

extern struct shm_shared_context *init_shared_context(const char *path, bool create);
extern void exit_shared_context(struct shm_shared_context *context);

extern void *mmap_arena(struct shm_shared_context *context, uint32_t index);
extern void munmap_arena(void *addr);

extern uint64_t malloc_arena(size_t size);
extern void free_arena(uint32_t index, uint32_t offset);
extern size_t check_arena(uint32_t index, uint32_t offset);

extern void lock_context();
extern void unlock_context();

static inline bool is_arena_valid(const struct shm_shared_context *context, uint32_t index) {
    return index < SHM_ARENA_MAX && context->arenas[index].type != ARENA_TYPE_EMPTY;
}

#endif
