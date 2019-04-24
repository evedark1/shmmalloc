#ifndef _SHM_TREE_H
#define _SHM_TREE_H

#include <stdint.h>
#include <assert.h>
#include "shm_malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct shm_tree_node {
    uint32_t rank;
    uint64_t root;
    uint64_t left;
    uint64_t right;
};

struct shm_pool {
    uint64_t root;
    uint32_t size;
};

struct shm_tree_addr {
    uint64_t pos;
    struct shm_tree_node *addr;
};

extern struct shm_tree_addr shm_tree_getaddr(uint64_t addr);
extern struct shm_tree_addr shm_tree_merge(struct shm_tree_addr tr1, struct shm_tree_addr tr2);

static inline void shm_tree_push(struct shm_pool *pool, uint64_t node)
{
    struct shm_tree_addr proot = shm_tree_getaddr(pool->root);
    struct shm_tree_addr pnode = shm_tree_getaddr(node);
    pool->root = shm_tree_merge(proot, pnode).pos;
    pool->size++;
}

static inline void shm_tree_pop(struct shm_pool *pool)
{
    assert(pool->size > 0);
    struct shm_tree_addr proot = shm_tree_getaddr(pool->root);
    struct shm_tree_addr left = shm_tree_getaddr(proot.addr->left);
    struct shm_tree_addr right = shm_tree_getaddr(proot.addr->right);
    struct shm_tree_addr r = shm_tree_merge(left, right);
    if(r.addr != NULL)
        r.addr->root = SHM_NULL;
    pool->root = r.pos;
    pool->size--;
}

static inline void shm_tree_remove(struct shm_pool *pool, uint64_t node)
{
    if(pool->root == node) {
        shm_tree_pop(pool);
        return;
    }

    struct shm_tree_addr pnode = shm_tree_getaddr(node);
    struct shm_tree_addr left = shm_tree_getaddr(pnode.addr->left);
    struct shm_tree_addr right = shm_tree_getaddr(pnode.addr->right);
    struct shm_tree_addr r = shm_tree_merge(left, right);

    struct shm_tree_addr root = shm_tree_getaddr(pnode.addr->root);
    if(r.addr != NULL)
        r.addr->root = root.pos;
    if(node == root.addr->left) {
        root.addr->left = r.pos;
    } else {
        root.addr->right = r.pos;
    }
    pool->size--;
}

#ifdef __cplusplus
}
#endif

#endif
