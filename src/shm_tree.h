#ifndef _SHM_TREE_H
#define _SHM_TREE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct shm_tree_node {
    uint32_t rank;
    uint64_t left;
    uint64_t right;
};

extern struct shm_tree_node *shm_tree_getaddr(uint64_t addr);

extern uint64_t shm_tree_merge(
        uint64_t tr1, struct shm_tree_node *pr1,
        uint64_t tr2, struct shm_tree_node *pr2);

static inline uint64_t shm_tree_push(uint64_t root, uint64_t node)
{
    struct shm_tree_node *proot = shm_tree_getaddr(root);
    struct shm_tree_node *pnode = shm_tree_getaddr(node);
    return shm_tree_merge(root, proot, node, pnode);
}

static inline uint64_t shm_tree_pop(uint64_t root)
{
    struct shm_tree_node *proot = shm_tree_getaddr(root);
    struct shm_tree_node *l = shm_tree_getaddr(proot->left);
    struct shm_tree_node *r = shm_tree_getaddr(proot->right);
    return shm_tree_merge(proot->left, l, proot->right, r);
}

#ifdef __cplusplus
}
#endif

#endif
