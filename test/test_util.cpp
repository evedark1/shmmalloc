#include "shm_util.h"
#include <gtest/gtest.h>

TEST(TestBitmap, Get) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(256)];
    memset(bitmap, 0, sizeof(bitmap));
    for(int i = 0; i < 256; i++) {
        EXPECT_FALSE(bitmap_get(bitmap, i));
    }
    memset(bitmap, 0xff, sizeof(bitmap));
    for(int i = 0; i < 256; i++) {
        EXPECT_TRUE(bitmap_get(bitmap, i));
    }

    memset(bitmap, 0, sizeof(bitmap));
    bitmap[0] = 0x10208081UL;
    EXPECT_TRUE(bitmap_get(bitmap, 0));
    EXPECT_TRUE(bitmap_get(bitmap, 7));
    EXPECT_TRUE(bitmap_get(bitmap, 15));
    EXPECT_TRUE(bitmap_get(bitmap, 21));
    EXPECT_TRUE(bitmap_get(bitmap, 28));
    bitmap[1] = 0x1021UL;
    EXPECT_TRUE(bitmap_get(bitmap, 64));
    EXPECT_TRUE(bitmap_get(bitmap, 69));
    EXPECT_TRUE(bitmap_get(bitmap, 76));
}

TEST(TestBitmap, SetClear) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(256)];
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_set(bitmap, 0);
    bitmap_set(bitmap, 7);
    bitmap_set(bitmap, 15);
    bitmap_set(bitmap, 21);
    bitmap_set(bitmap, 28);
    EXPECT_TRUE(bitmap[0] == 0x10208081UL);

    bitmap_clear(bitmap, 0);
    bitmap_clear(bitmap, 7);
    bitmap_clear(bitmap, 15);
    bitmap_clear(bitmap, 21);
    bitmap_clear(bitmap, 28);
    EXPECT_TRUE(bitmap[0] == 0);

    bitmap_set(bitmap, 64);
    bitmap_set(bitmap, 69);
    bitmap_set(bitmap, 76);
    EXPECT_TRUE(bitmap[1] == 0x1021UL);

    bitmap_clear(bitmap, 64);
    bitmap_clear(bitmap, 69);
    bitmap_clear(bitmap, 76);
    EXPECT_TRUE(bitmap[1] == 0);
}

TEST(TestBitmap, BitmapFfu) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(256)];
    const uint32_t l = 256;
    memset(bitmap, 0xff, sizeof(bitmap));

    EXPECT_EQ(bitmap_ffu(bitmap, l), l);

    bitmap_clear(bitmap, 255);
    EXPECT_EQ(bitmap_ffu(bitmap, l), 255);

    bitmap_clear(bitmap, 100);
    EXPECT_EQ(bitmap_ffu(bitmap, l), 100);

    bitmap_clear(bitmap, 1);
    EXPECT_EQ(bitmap_ffu(bitmap, l), 1);

    bitmap_clear(bitmap, 0);
    EXPECT_EQ(bitmap_ffu(bitmap, l), 0);
}

TEST(TestBitmap, BitmapFfs) {
    bitmap_t bitmap[BITMAP_BITS2GROUPS(256)];
    const uint32_t l = 256;
    memset(bitmap, 0, sizeof(bitmap));

    EXPECT_EQ(bitmap_ffs(bitmap, l), l);
    EXPECT_EQ(bitmap_ffsp(bitmap, 0, l), l);

    bitmap_set(bitmap, 255);
    EXPECT_EQ(bitmap_ffs(bitmap, l), 255);
    EXPECT_EQ(bitmap_ffsp(bitmap, 0, l), 255);
    EXPECT_EQ(bitmap_ffsp(bitmap, 100, l), 255);

    bitmap_set(bitmap, 100);
    EXPECT_EQ(bitmap_ffs(bitmap, l), 100);
    EXPECT_EQ(bitmap_ffsp(bitmap, 0, l), 100);
    EXPECT_EQ(bitmap_ffsp(bitmap, 101, l), 255);

    bitmap_set(bitmap, 0);
    EXPECT_EQ(bitmap_ffs(bitmap, l), 0);
    EXPECT_EQ(bitmap_ffsp(bitmap, 0, l), 0);
    EXPECT_EQ(bitmap_ffsp(bitmap, 1, l), 100);
}

TEST(TestUtil, AlignSize) {
    EXPECT_EQ(align_size(0, 8), 0);
    EXPECT_EQ(align_size(1, 8), 8);
    EXPECT_EQ(align_size(8, 8), 8);
    EXPECT_EQ(align_size(60, 8), 64);
    EXPECT_EQ(align_size(64, 8), 64);
}
