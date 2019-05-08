#ifndef _CHUNK_MEDIUM_H_
#define _CHUNK_MEDIUM_H_

#include <assert.h>
#include "shm_type.h"

extern uint32_t get_medium_max_size(struct chunk_header *chunk);

extern void init_chunk_medium(struct chunk_header *chunk);
extern uint64_t malloc_chunk_medium(struct chunk_header *chunk, size_t len);
extern bool free_chunk_medium(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk_medium(struct chunk_header *chunk, uint32_t offset);

#endif
