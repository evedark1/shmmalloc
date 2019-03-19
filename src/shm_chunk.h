#ifndef _SHM_CHUNK_H_
#define _SHM_CHUNK_H_

#include "shm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

// run config info, start from 1
extern const struct run_config run_config_list[RUN_CONFIG_SIZE];
extern uint32_t find_run_config(size_t size);

extern void init_chunk(uint64_t pos, uint32_t type);
extern uint64_t malloc_chunk_small(struct chunk_header *chunk, uint32_t runtype);
extern void free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif

