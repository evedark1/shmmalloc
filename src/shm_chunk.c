#include "shm_chunk.h"
#include <string.h>
#include "shm_logger.h"
#include "shm_util.h"
#include "shm_malloc_inc.h"

static_assert(sizeof(struct chunk_header) <= SHM_RUN_UNIT_SIZE, "chunk header too big");

// define rbtree
typedef struct chunk_tree_value_t {
    uint32_t index;
    uint32_t maxsize;	// small : 0 or 1, medium: max size left
} chunk_tree_value;

#define RB_NAME(name) chunk_##name
#define RB_POINTER uint64_t
#define RB_KEY chunk_tree_value
#define RB_NULL SHM_NULL
#define RB_GET_NODE(p) get_or_update_addr(p)

static inline int chunk_tree_compare(RB_KEY a, RB_KEY b) {
    if(a.maxsize == b.maxsize)
        return a.index - b.index;
    return a.maxsize - b.maxsize;
}
#define RB_COMPARE(a, b) chunk_tree_compare(a, b)
#include "rbtree.h"
static_assert(sizeof(chunk_tree) < sizeof(struct context_chunk_holder), "chunk rbtree too big");
static_assert(sizeof(chunk_node) < sizeof(struct chunk_node_holder), "chunk rbtree node too big");

void init_chunk(struct chunk_header *chunk, uint64_t pos, uint32_t type)
{
    struct shm_shared_context *context = local_context.shared_context;
    memset(chunk, 0, sizeof(struct chunk_header));
    chunk->pos = pos;
    chunk->type = type;

    chunk_node *node = (chunk_node*)chunk;
    node->s = pos;
    node->v.index = pos2index(pos);
    if(type == CHUNK_TYPE_SMALL) {
        init_chunk_small(chunk);
        node->v.maxsize = 1;
        rbtree_insert((chunk_tree*)&context->chunk_small_pool, node);
    } else { // type == CHUNK_TYPE_MEDIUM
        init_chunk_medium(chunk);
        node->v.maxsize = get_medium_max_size(chunk);
        rbtree_insert((chunk_tree*)&context->chunk_medium_pool, node);
    }
}

struct chunk_header *get_avaliable_chunk(uint32_t type, size_t size)
{
    struct shm_shared_context *context = local_context.shared_context;
    struct chunk_header *chunk = NULL;
    chunk_tree_value v;

    if(type == CHUNK_TYPE_SMALL) {
        v.index = 0;
        v.maxsize = 1;
        chunk = (struct chunk_header*)rbtree_lower((chunk_tree*)&context->chunk_small_pool, v);
    } else {
        v.index = 0;
        v.maxsize = size;
        chunk = (struct chunk_header*)rbtree_lower((chunk_tree*)&context->chunk_medium_pool, v);
    }
    return chunk;
}

static void delete_chunk(struct chunk_header *chunk)
{
    struct shm_shared_context *context = local_context.shared_context;
    if(chunk->type == CHUNK_TYPE_SMALL) {
        rbtree_delete((chunk_tree*)&context->chunk_small_pool, (chunk_node*)chunk);
    } else {
        rbtree_delete((chunk_tree*)&context->chunk_medium_pool, (chunk_node*)chunk);
    }
    chunk->type = CHUNK_TYPE_EMPTY;
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
    if(check)
        delete_chunk(chunk);
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
        ret = check_chunk_medium(chunk, offset);
        break;
    default:
        logWarning("free chunk type invalid %d", chunk->type);
        break;
    }
    return ret;
}

void update_chunk_pool(struct chunk_header *chunk, uint32_t new_max)
{
    struct shm_shared_context *context = local_context.shared_context;
    chunk_node *node = (chunk_node*)chunk;
    chunk_tree *tree;
    if(chunk->type == CHUNK_TYPE_SMALL)
        tree = (chunk_tree*)&context->chunk_small_pool;
    else
        tree = (chunk_tree*)&context->chunk_medium_pool;

    rbtree_delete(tree, node);
    node->v.maxsize = new_max;
    rbtree_insert(tree, node);
}
