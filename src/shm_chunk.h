#ifndef _SHM_CHUNK_H_
#define _SHM_CHUNK_H_

#include <assert.h>
#include "shm_type.h"
#include "chunk_small.h"
#include "chunk_medium.h"

extern void init_chunk(uint64_t pos, uint32_t type);
extern bool free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

#endif
