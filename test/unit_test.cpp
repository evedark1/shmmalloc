#include <gtest/gtest.h>
#include <vector>

#define RB_POINTER void*
#define RB_VALUE int
#define RB_NULL NULL
#define RB_GET_NODE(p) (struct rbtree_node*)(p)
#define RB_COMPARE(a, b) (a - b)
#include "rbtree.h"

TEST(TestRBTree, Insert) {
    int v[10] = {5,2,6,3,1,0,9,4,8,7};
    std::vector<int> r;

    struct rbtree tree;
    rbtree_init(&tree);
    for(int i = 0; i < 10; i++) {
        struct rbtree_node *node = (struct rbtree_node*)malloc(sizeof(struct rbtree_node));
        node->s = node;
        node->v = v[i];
        rbtree_insert(&tree, node);
    }
    EXPECT_EQ(tree.size, 10);

    struct rbtree_node *node = RB_GET_NODE(tree.min);
    EXPECT_EQ(node->v, 0);
    while(node != NULL) {
        r.push_back(node->v);
        node = rbtree_next(&tree, node);
    }

    std::sort(v, v + 10);
    EXPECT_TRUE(std::equal(r.begin(), r.end(), v));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
