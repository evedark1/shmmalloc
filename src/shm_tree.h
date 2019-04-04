#ifndef _SHM_TREE_H
#define _SHM_TREE_H

#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

struct shm_tree_node {
    uint32_t rank;
    uint64_t left;
    uint64_t right;
};

struct shm_pool {
    uint64_t root;
    uint32_t size;
};

extern struct shm_tree_node *shm_tree_getaddr(uint64_t addr);

extern uint64_t shm_tree_merge(
        uint64_t tr1, struct shm_tree_node *pr1,
        uint64_t tr2, struct shm_tree_node *pr2);

static inline void shm_tree_push(struct shm_pool *pool, uint64_t node)
{
    struct shm_tree_node *proot = shm_tree_getaddr(pool->root);
    struct shm_tree_node *pnode = shm_tree_getaddr(node);
    pool->root = shm_tree_merge(pool->root, proot, node, pnode);
    pool->size++;
}

static inline void shm_tree_pop(struct shm_pool *pool)
{
    assert(pool->size > 0);
    struct shm_tree_node *proot = shm_tree_getaddr(pool->root);
    struct shm_tree_node *l = shm_tree_getaddr(proot->left);
    struct shm_tree_node *r = shm_tree_getaddr(proot->right);
    pool->root = shm_tree_merge(proot->left, l, proot->right, r);
    pool->size--;
}

#ifdef __cplusplus
}
#endif

#endif
