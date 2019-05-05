#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <stdint.h>

struct rbtree_node *rb_get_node(uint16_t p);

#define RB_POINTER uint16_t
#define RB_VALUE int
#define RB_NULL 0xffff 
#define RB_GET_NODE(p) rb_get_node(p)
#define RB_COMPARE(a, b) (a - b)
#include "rbtree.h"

static struct rbtree_node *base = NULL;
struct rbtree_node *rb_get_node(uint16_t p) {
    return base + p;
}

void setTestTree(struct rbtree *tree, int *v, struct rbtree_node *nodes, int len) {
    base = nodes;
    rbtree_init(tree);
    for(int i = 0; i < len; i++) {
        struct rbtree_node *node = nodes + i;
        node->s = i;
        node->v = v[i];
        rbtree_insert(tree, node);
    }
}

TEST(TestRBTree, Delete) {
    int v[10] = {5,2,6,3,1,0,9,4,8,7};
    struct rbtree_node nodes[10];
    struct rbtree tree;
    setTestTree(&tree, v, nodes, 10);
    EXPECT_EQ(tree.size, 10);

    struct rbtree_node *d;
    d = rbtree_find(&tree, 3);
    EXPECT_TRUE(d != NULL);
    rbtree_delete(&tree, d);
    EXPECT_TRUE(rbtree_find(&tree, 3) == NULL);
    EXPECT_EQ(tree.size, 9);
}


TEST(TestRBTree, Sorted) {
    int v[10] = {9,8,7,6,5,4,3,2,1,0};
    std::vector<int> r;
    struct rbtree_node nodes[10];
    struct rbtree tree;
    setTestTree(&tree, v, nodes, 10);

    struct rbtree_node *node = rbtree_min(&tree);
    EXPECT_EQ(node->v, 0);
    while(node != NULL) {
        r.push_back(node->v);
        node = rbtree_next(&tree, node);
    }

    std::sort(v, v + 10);
    EXPECT_TRUE(std::equal(r.begin(), r.end(), v));
}

TEST(TestRBTree, Find) {
    int v[5] = {5,3,1,9,7};
    struct rbtree_node nodes[5];
    struct rbtree tree;
    setTestTree(&tree, v, nodes, 5);

    // test find
    for(int i = 0; i < 5; i++) {
        struct rbtree_node *node = rbtree_find(&tree, v[i]);
        EXPECT_EQ(v[i], node->v);
    }
    EXPECT_TRUE(rbtree_find(&tree, 12) == NULL);
    EXPECT_TRUE(rbtree_find(&tree, -2) == NULL);

    // test lower
    struct rbtree_node *n;
    n = rbtree_lower(&tree, 0);
    EXPECT_TRUE(n != NULL);
    EXPECT_EQ(1, n->v);

    n = rbtree_lower(&tree, 2);
    EXPECT_TRUE(n != NULL);
    EXPECT_EQ(3, n->v);

    n = rbtree_lower(&tree, 3);
    EXPECT_TRUE(n != NULL);
    EXPECT_EQ(3, n->v);

    n = rbtree_lower(&tree, 8);
    EXPECT_TRUE(n != NULL);
    EXPECT_EQ(9, n->v);

    n = rbtree_lower(&tree, 9);
    EXPECT_TRUE(n != NULL);
    EXPECT_EQ(9, n->v);

    n = rbtree_lower(&tree, 10);
    EXPECT_TRUE(n == NULL);
}