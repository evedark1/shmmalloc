#include "shm_chunk.h"
#include <string.h>
#include "shm_logger.h"
#include "shm_util.h"
#include "shm_malloc_inc.h"

static_assert(sizeof(struct chunk_header) <= SHM_RUN_UNIT_SIZE, "chunk header too big");

void init_chunk(uint64_t pos, uint32_t type)
{
    struct chunk_header *chunk = get_or_update_addr(pos);
    memset(chunk, 0, sizeof(struct chunk_header));
    chunk->pos = pos;
    chunk->type = type;
    if(type == CHUNK_TYPE_SMALL) {
        init_chunk_small(chunk);
    } else { // type == CHUNK_TYPE_MEDIUM
        init_chunk_medium(chunk);
    }
}

bool free_chunk(struct chunk_header *chunk, uint32_t offset)
{
    bool check = false;
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        check = free_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        check = free_chunk_medium(chunk, offset);
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
    return check;
}

size_t check_chunk(struct chunk_header *chunk, uint32_t offset)
{
    size_t ret = 0;
    switch(chunk->type) {
    case CHUNK_TYPE_SMALL:
        ret = check_chunk_small(chunk, offset);
        break;
    case CHUNK_TYPE_MEDIUM:
        //TODO: chunk medium
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
    return ret;
}
