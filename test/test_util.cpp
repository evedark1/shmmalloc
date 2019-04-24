#include "shm_util.h"
#include <gtest/gtest.h>

TEST(TestBitmap, SetGet) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(64)];
    memset(bitmap, 0, sizeof(bitmap));
    EXPECT_FALSE(bitmap_get(bitmap, 0));
    EXPECT_FALSE(bitmap_get(bitmap, 12));
    EXPECT_FALSE(bitmap_get(bitmap, 63));

    bitmap_set(bitmap, 0);
    bitmap_set(bitmap, 12);
    bitmap_set(bitmap, 63);
    EXPECT_TRUE(bitmap_get(bitmap, 0));
    EXPECT_TRUE(bitmap_get(bitmap, 12));
    EXPECT_TRUE(bitmap_get(bitmap, 63));

    bitmap_clear(bitmap, 12);
    bitmap_clear(bitmap, 13);
    EXPECT_FALSE(bitmap_get(bitmap, 12));
    EXPECT_FALSE(bitmap_get(bitmap, 13));
}

TEST(TestBitmap, BitmapFfu) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(64)];
    memset(bitmap, 0, sizeof(bitmap));
    EXPECT_EQ(bitmap_ffu(bitmap, 64), 0);

    bitmap_set(bitmap, 0);
    EXPECT_EQ(bitmap_ffu(bitmap, 64), 1);

    memset(bitmap, 0xff, sizeof(bitmap));
    EXPECT_TRUE(bitmap_ffu(bitmap, 64) >= 64);

    bitmap_clear(bitmap, 10);
    EXPECT_EQ(bitmap_ffu(bitmap, 64), 10);
}

TEST(TestUtil, AlignSize) {
    EXPECT_EQ(align_size(0, 8), 0);
    EXPECT_EQ(align_size(1, 8), 8);
    EXPECT_EQ(align_size(8, 8), 8);
    EXPECT_EQ(align_size(60, 8), 64);
    EXPECT_EQ(align_size(64, 8), 64);
}
