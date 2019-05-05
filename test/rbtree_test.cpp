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

void dumpTreeMid(RB_POINTER p) {
    if(p == RB_NULL)
        return;
    struct rbtree_node *n = RB_GET_NODE(p);
    dumpTreeMid(n->left);
    std::cout << n->v << ":" << int(n->color) << " ";
    dumpTreeMid(n->right);
}

void dumpTreePre(RB_POINTER p) {
    if(p == RB_NULL)
        return;
    struct rbtree_node *n = RB_GET_NODE(p);
    std::cout << n->v << ":" << int(n->color) << " ";
    dumpTreePre(n->left);
    dumpTreePre(n->right);
}

void dumpTree(struct rbtree *tree) {
    std::cout << "pre: ";
    dumpTreePre(tree->root);
    std::cout << std::endl;

    std::cout << "mid: ";
    dumpTreeMid(tree->root);
    std::cout << std::endl;
}

bool checkTreeOrder(struct rbtree_node *node) {
    if(node->left != RB_NULL) {
        struct rbtree_node *l = RB_GET_NODE(node->left);
        if(RB_COMPARE(l->v, node->v) < 0)
            return checkTreeOrder(l);
        else
            return false;
    }
    if(node->right != RB_NULL) {
        struct rbtree_node *r = RB_GET_NODE(node->right);
        if(RB_COMPARE(node->v, r->v) < 0)
            return checkTreeOrder(r);
        else
            return false;
    }
    return true;
}

bool checkTreeDeep(struct rbtree_node *node, int deep, int target) {
    if(node->color == RB_BLACK)
        deep++;

    if(node->left == RB_NULL && node->right == RB_NULL) {
        return deep == target;
    }

    if(node->left != RB_NULL) {
        struct rbtree_node *l = RB_GET_NODE(node->left);
        if(!checkTreeDeep(l, deep, target))
            return false;
    }
    if(node->right != RB_NULL) {
        struct rbtree_node *r = RB_GET_NODE(node->right);
        if(!checkTreeDeep(r, deep, target))
            return false;
    }
    return true;
}

bool checkTree(struct rbtree *tree) {
    if(tree->root == RB_NULL)
        return true;
    struct rbtree_node *root = RB_GET_NODE(tree->root);
    if(!checkTreeOrder(root)) {
        std::cout << "tree order error:" << std::endl;
        dumpTree(tree);
        return false;
    }

    int deep = 0;
    struct rbtree_node *node = root;
    while(node != NULL) {
        if(node->color == RB_BLACK)
            deep++;
        node = rbtree_check_node(node->left);
    }
    if(!checkTreeDeep(root, 0, deep)) {
        std::cout << "tree deep error: " << deep << std::endl;
        dumpTree(tree);
        return false;
    }
    return true;
}

TEST(TestRBTree, Insert) {
    int v[15] = {5,14,2,6,15,3,1,13,12,10,9,4,11,8,7};
    struct rbtree_node nodes[15];
    struct rbtree tree;

    base = nodes;
    rbtree_init(&tree);
    for(int i = 0; i < 15; i++) {
        struct rbtree_node *node = nodes + i;
        node->s = i;
        node->v = v[i];
        rbtree_insert(&tree, node);
        ASSERT_TRUE(checkTree(&tree));
        EXPECT_EQ(tree.size, i + 1);
    }
}

TEST(TestRBTree, Delete) {
    int v[15] = {5,14,2,6,15,3,1,13,12,10,9,4,11,8,7};
    struct rbtree_node nodes[15];
    struct rbtree tree;
    setTestTree(&tree, v, nodes, 15);

    for(int i = 0; i < 15; i++) {
        int t = v[i];
        struct rbtree_node *d;
        d = rbtree_find(&tree, t);
        EXPECT_TRUE(d != NULL);
        rbtree_delete(&tree, d);

        EXPECT_TRUE(rbtree_find(&tree, t) == NULL);
        ASSERT_TRUE(checkTree(&tree));
        EXPECT_EQ(tree.size, 14 - i);
    }
}


TEST(TestRBTree, Sorted) {
    int v[10] = {9,8,7,6,5,4,3,2,1,0};
    std::vector<int> r;
    struct rbtree_node nodes[10];
    struct rbtree tree;
    setTestTree(&tree, v, nodes, 10);
    ASSERT_TRUE(checkTree(&tree));

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