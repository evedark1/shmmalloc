#ifndef _SHM_CHUNK_H_
#define _SHM_CHUNK_H_

#include "shm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

struct run_config {
    uint32_t elemsize;
    uint32_t regsize;
};

#define CHUNK_SMALL_LIMIT 2048			// 2 KB
#define CHUNK_MEDIUM_LIMIT (1024 * 1024) // 1 MB

#define RUN_CONFIG_SIZE 15
// run config info, start from 1
extern const struct run_config *find_run_config(size_t size);

extern void free_chunk(struct chunk_header *chunk, uint32_t offset);
extern size_t check_chunk(struct chunk_header *chunk, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif

