#include "shm_tree.h"
#include "shm_malloc_inc.h"

struct shm_tree_addr shm_tree_getaddr(uint64_t addr)
{
    struct shm_tree_addr r;
    r.pos = addr;
    r.addr = get_or_update_addr(addr);
    return r;
}

// leftist tree merge
struct shm_tree_addr shm_tree_merge(struct shm_tree_addr tr1, struct shm_tree_addr tr2)
{
    if(tr1.pos == SHM_NULL)
        return tr2;
    if(tr2.pos == SHM_NULL)
        return tr1;

    if(tr1.pos > tr2.pos) {
        struct shm_tree_addr t = tr1;
        tr1 = tr2;
        tr2 = t;
    }

    struct shm_tree_addr rnode = shm_tree_merge(shm_tree_getaddr(tr1.addr->right), tr2);
    rnode.addr->root = tr1.pos;
    tr1.addr->right = rnode.pos;

    if(tr1.addr->left == SHM_NULL) {
        tr1.addr->left = tr1.addr->right;
        tr1.addr->right = SHM_NULL;
    } else {
        struct shm_tree_addr lnode = shm_tree_getaddr(tr1.addr->left);
        if(lnode.addr->rank < rnode.addr->rank) {
            uint64_t t = tr1.addr->left;
            tr1.addr->left = tr1.addr->right;
            tr1.addr->right = t;

            tr1.addr->rank = lnode.addr->rank + 1;
        } else {
            tr1.addr->rank = rnode.addr->rank + 1;
        }
    }
    return tr1;
}
