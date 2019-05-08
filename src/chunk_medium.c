#include "chunk_medium.h"
#include <string.h>
#include "shm_logger.h"
#include "shm_chunk.h"
#define CHUNK_MEDIUM_UNITS (SHM_CHUNK_UNIT_SIZE / SHM_PAGE_SIZE)

// define rbtree
typedef struct medium_tree_value_t {
    uint16_t size;
    uint16_t index;
} medium_tree_value;

#define RB_POINTER uint16_t
#define RB_KEY medium_tree_value
#define RB_NULL 0

struct chunk_header *medium_chunk_base = NULL;
static inline void *medium_tree_get(RB_POINTER p) {
    return (char*)medium_chunk_base + p * SHM_PAGE_SIZE;
}
#define RB_GET_NODE(p) medium_tree_get(p)

static inline int medium_tree_compare(RB_KEY a, RB_KEY b) {
    uint16_t r = a.size - b.size;
    if(r != 0)
        return r;
    return a.index - b.index;
}
#define RB_COMPARE(a, b) medium_tree_compare(a, b)
#include "rbtree.h"

typedef struct chunk_medium_detial_t {
    uint16_t max_size;
    uint16_t free_size;
    struct rbtree tree;
    bitmap_t bitmap_type[BITMAP_BITS2GROUPS(CHUNK_MEDIUM_UNITS)];	// 0 free, 1 used
    bitmap_t bitmap_up[BITMAP_BITS2GROUPS(CHUNK_MEDIUM_UNITS)];
    bitmap_t bitmap_down[BITMAP_BITS2GROUPS(CHUNK_MEDIUM_UNITS)];
} chunk_medium_detial;
static_assert(sizeof(chunk_medium_detial) < sizeof(struct chunk_detial_holder), "chunk medium too big");

static void set_medium_unit(chunk_medium_detial *detial, uint16_t p)
{
    uint16_t q = (CHUNK_MEDIUM_UNITS - 1) - p;
    assert(!bitmap_get(detial->bitmap_up, p));
    assert(!bitmap_get(detial->bitmap_down, q));
    bitmap_set(detial->bitmap_up, p);
    bitmap_set(detial->bitmap_down, q);
}

static bool get_medium_unit(chunk_medium_detial *detial, uint16_t p)
{
    bool r = bitmap_get(detial->bitmap_up, p);
    assert(r == bitmap_get(detial->bitmap_down, ((CHUNK_MEDIUM_UNITS - 1) - p)));
    return r;
}

static void clear_medium_unit(chunk_medium_detial *detial, uint16_t p)
{
    uint16_t q = (CHUNK_MEDIUM_UNITS - 1) - p;
    assert(bitmap_get(detial->bitmap_up, p));
    assert(bitmap_get(detial->bitmap_down, q));
    bitmap_clear(detial->bitmap_up, p);
    bitmap_clear(detial->bitmap_down, q);
}

static uint16_t find_next_medium_unit(chunk_medium_detial *detial, uint16_t p)
{
    return bitmap_ffsp(detial->bitmap_up, p, CHUNK_MEDIUM_UNITS);
}

static uint16_t find_prev_medium_unit(chunk_medium_detial *detial, uint16_t p)
{
    uint16_t q = (CHUNK_MEDIUM_UNITS - 1) - p;
    q = bitmap_ffsp(detial->bitmap_down, q, CHUNK_MEDIUM_UNITS);
    if(q != CHUNK_MEDIUM_UNITS)
        return (CHUNK_MEDIUM_UNITS - 1) - q;
    else
        return CHUNK_MEDIUM_UNITS;
}

uint32_t get_medium_max_size(struct chunk_header *chunk)
{
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    return detial->max_size * SHM_PAGE_SIZE;
}

void init_chunk_medium(struct chunk_header *chunk)
{
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    medium_chunk_base = chunk;
    // first page used by chunk_header
    detial->max_size = CHUNK_MEDIUM_UNITS - 1;
    detial->free_size = CHUNK_MEDIUM_UNITS - 1;
    // init free tree
    rbtree_init(&detial->tree);
    struct rbtree_node *first_node = medium_tree_get(1);
    first_node->v.size = CHUNK_MEDIUM_UNITS - 1;
    first_node ->v.index = 1;
    first_node->s = 1;
    rbtree_insert(&detial->tree, first_node);
    // init bitmap
    set_medium_unit(detial, first_node->s);
}

static void check_chunk_medium_max(struct chunk_header *chunk, uint16_t s)
{
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    if(s == 0) {
        struct rbtree_node *n = rbtree_max(&detial->tree);
        s = (n == NULL) ? 0 : n->v.size;
    }
    if(detial->max_size != s) {
        update_medium_chunk(chunk, s * SHM_PAGE_SIZE);
        detial->max_size = s;
    }
}

uint64_t malloc_chunk_medium(struct chunk_header *chunk, size_t len)
{
    assert(chunk->type == CHUNK_TYPE_MEDIUM);
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    medium_chunk_base = chunk;
    uint16_t size = align_size(len, SHM_PAGE_SIZE) / SHM_PAGE_SIZE;
    uint64_t ret = SHM_NULL;

    // find first match free node
    medium_tree_value key = {size, 0};
    struct rbtree_node *n = rbtree_lower(&detial->tree, key);
    assert(n != NULL);
    assert(n->v.size >= size && n->v.index == n->s);

    // adjust free tree
    if(n->v.size > size) {
        uint16_t next_index = n->v.index + size;
        uint16_t next_size = n->v.size - size;
        assert(next_index + next_size <= CHUNK_MEDIUM_UNITS);

        struct rbtree_node *next_unit = medium_tree_get(next_index);
        next_unit->v.size = next_size;
        next_unit->v.index = next_index;
        next_unit->s = next_index;
        rbtree_insert(&detial->tree, next_unit);
        set_medium_unit(detial, next_index);
    }
    ret = chunk->pos + n->s * SHM_PAGE_SIZE;
    rbtree_delete(&detial->tree, n);
    bitmap_set(detial->bitmap_type, n->s);

    // change medium chunk info
    detial->free_size -= size;
    if(n->v.size >= detial->max_size)
        check_chunk_medium_max(chunk, 0);

    // clear node info for safe
    n->v.size = 0;
    n->v.index = 0;
    n->s = 0;
    return ret;
}

bool free_chunk_medium(struct chunk_header *chunk, uint32_t offset)
{
    assert(chunk->type == CHUNK_TYPE_MEDIUM);
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    medium_chunk_base = chunk;

    // check free offset
    uint16_t curr_index = offset / SHM_PAGE_SIZE;
    if(curr_index >= CHUNK_MEDIUM_UNITS || offset % SHM_PAGE_SIZE != 0) {
        logNotice("free chunk medium offset error");
        return false;
    }
    if(!bitmap_get(detial->bitmap_type, curr_index)) {
        logNotice("free chunk medium not used");
        return false;
    }
    assert(get_medium_unit(detial, curr_index));
    clear_medium_unit(detial, curr_index);
    bitmap_clear(detial->bitmap_type, curr_index);

    // find next unit
    uint16_t next_index = find_next_medium_unit(detial, curr_index);
    uint16_t curr_size = next_index - curr_index;
    medium_tree_value insert;
    insert.index = curr_index;
    insert.size = curr_size;

    // merge current unit and next unit
    if(next_index != CHUNK_MEDIUM_UNITS && !bitmap_get(detial->bitmap_type, next_index)) {
        struct rbtree_node *n = medium_tree_get(next_index);
        assert(n->s == next_index && n->v.size);
        insert.size += n->v.size;

        rbtree_delete(&detial->tree, n);
        clear_medium_unit(detial, next_index);
        n->v.size = 0;
        n->v.index = 0;
        n->s = 0;
    }

    // merge current unit and previous unit
    uint16_t prev_index = find_prev_medium_unit(detial, curr_index);
    if(prev_index != CHUNK_MEDIUM_UNITS && !bitmap_get(detial->bitmap_type, prev_index)) {
        struct rbtree_node *n = medium_tree_get(prev_index);
        assert(n->s == prev_index && n->v.size);
        insert.index = prev_index;
        insert.size += n->v.size;

        rbtree_delete(&detial->tree, n);
        clear_medium_unit(detial, prev_index);
        n->v.size = 0;
        n->v.index = 0;
        n->s = 0;
    }

    // add new free unit
    struct rbtree_node *curr = medium_tree_get(insert.index);
    curr->v = insert;
    curr->s = insert.index;
    rbtree_insert(&detial->tree, curr);
    set_medium_unit(detial, insert.index);

    // change medium chunk info
    detial->free_size += curr_size;
    if(curr->v.size >= detial->max_size)
        check_chunk_medium_max(chunk, curr->v.size);

    return detial->free_size == CHUNK_MEDIUM_UNITS - 1;
}

size_t check_chunk_medium(struct chunk_header *chunk, uint32_t offset)
{
    assert(chunk->type == CHUNK_TYPE_MEDIUM);
    chunk_medium_detial *detial = (chunk_medium_detial*)(&chunk->detial);
    // check free offset
    uint16_t curr_index = offset / SHM_PAGE_SIZE;
    if(curr_index >= CHUNK_MEDIUM_UNITS || offset % SHM_PAGE_SIZE != 0) {
        logNotice("check chunk medium offset error");
        return 0;
    }
    if(!bitmap_get(detial->bitmap_type, curr_index)) {
        logNotice("check chunk medium not used");
        return 0;
    }
    assert(get_medium_unit(detial, curr_index));

    // find next unit
    uint16_t next_index = find_next_medium_unit(detial, curr_index);
    uint16_t curr_size = next_index - curr_index;
    return curr_size * SHM_PAGE_SIZE;
}
