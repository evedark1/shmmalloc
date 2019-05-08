#ifndef _SHM_CHUNK_H_
#define _SHM_CHUNK_H_

#include <assert.h>
#include "shm_type.h"
#include "chunk_small.h"
#include "chunk_medium.h"

extern void init_chunk(uint64_t pos, uint32_t type);
extern uint64_t get_avaliable_chunk(uint32_t type, size_t size);
extern bool free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

extern void update_medium_chunk(struct chunk_header *chunk, uint32_t new_max);

#endif
