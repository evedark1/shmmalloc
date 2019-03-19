#include "shm_chunk.h"
#include <gtest/gtest.h>

const struct run_config *find_conf(size_t size) {
    uint32_t idx = find_run_config(size);
    return &run_config_list[idx];
}

TEST(TestChunk, FindRunConfig) {
    EXPECT_EQ(find_conf(1)->elemsize, 8);
    EXPECT_EQ(find_conf(8)->elemsize, 8);
    EXPECT_EQ(find_conf(12)->elemsize, 16);
    EXPECT_EQ(find_conf(16)->elemsize, 16);
    EXPECT_EQ(find_conf(50)->elemsize, 64);
    EXPECT_EQ(find_conf(65)->elemsize, 96);
    EXPECT_EQ(find_conf(96)->elemsize, 96);

    EXPECT_EQ(find_conf(97)->elemsize, 128);
    EXPECT_EQ(find_conf(128)->elemsize, 128);
    EXPECT_EQ(find_conf(180)->elemsize, 192);
    EXPECT_EQ(find_conf(200)->elemsize, 256);
    EXPECT_EQ(find_conf(512)->elemsize, 512);

    EXPECT_EQ(find_conf(520)->elemsize, 768);
    EXPECT_EQ(find_conf(1000)->elemsize, 1024);
    EXPECT_EQ(find_conf(1025)->elemsize, 2048);
    EXPECT_EQ(find_conf(2048)->elemsize, 2048);
}
