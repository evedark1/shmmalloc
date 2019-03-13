#include "shm_chunk.h"
#include <gtest/gtest.h>

TEST(TestChunk, FindRunConfig) {
    EXPECT_EQ(find_run_config(1)->elemsize, 8);
    EXPECT_EQ(find_run_config(8)->elemsize, 8);
    EXPECT_EQ(find_run_config(12)->elemsize, 16);
    EXPECT_EQ(find_run_config(16)->elemsize, 16);
    EXPECT_EQ(find_run_config(50)->elemsize, 64);
    EXPECT_EQ(find_run_config(65)->elemsize, 96);
    EXPECT_EQ(find_run_config(96)->elemsize, 96);

    EXPECT_EQ(find_run_config(97)->elemsize, 128);
    EXPECT_EQ(find_run_config(128)->elemsize, 128);
    EXPECT_EQ(find_run_config(180)->elemsize, 192);
    EXPECT_EQ(find_run_config(200)->elemsize, 256);
    EXPECT_EQ(find_run_config(512)->elemsize, 512);

    EXPECT_EQ(find_run_config(520)->elemsize, 768);
    EXPECT_EQ(find_run_config(1000)->elemsize, 1024);
    EXPECT_EQ(find_run_config(1025)->elemsize, 2048);
    EXPECT_EQ(find_run_config(2048)->elemsize, 2048);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
