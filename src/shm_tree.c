#include "shm_tree.h"
#include "shm_malloc.h"
#include "shm_malloc_inc.h"

struct shm_tree_node *shm_tree_getaddr(uint64_t addr)
{
    return get_or_update_addr(addr);
}

uint64_t shm_tree_merge(
        uint64_t tr1, struct shm_tree_node *pr1,
        uint64_t tr2, struct shm_tree_node *pr2)
{
    if(tr1 == SHM_NULL)
        return tr2;
    if(tr2 == SHM_NULL)
        return tr1;

    if(tr1 > tr2) {
        uint64_t t = tr1;
        tr1 = tr2;
        tr2 = t;
    }

    struct shm_tree_node *rnode = shm_tree_getaddr(pr1->right);
    pr1->right = shm_tree_merge(pr1->right, rnode, tr2, pr2);

    if(pr1->left == SHM_NULL) {
        pr1->left = pr1->right;
        pr1->right = SHM_NULL;
    } else {
        struct shm_tree_node *lnode = shm_tree_getaddr(pr1->right);
        if(lnode->rank < rnode->rank) {
            uint64_t t = pr1->left;
            pr1->left = pr1->right;
            pr1->right = t;

            pr1->rank = lnode->rank + 1;
        } else {
            pr1->rank = rnode->rank + 1;
        }
    }
    return tr1;
}
