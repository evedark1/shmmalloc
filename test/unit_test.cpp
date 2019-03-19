#include "shm_op_wrapper.h"
#include <gtest/gtest.h>

TEST(TestShm, Mmap) {
    const char *path = "/tmp/shm_test";
    key_t key;
    int r;
    r = shm_get_key(path, 0, &key);
    ASSERT_TRUE(r == 0);

    int shmid = 0;
    int err = 0;
    void *addr = shm_mmap(key, 8*1024*1024, true, &shmid, &err);
    ASSERT_TRUE(addr != NULL);
    ASSERT_TRUE(shmid > 0);

    memset(addr, 0xab, 8*1024*1024);
    unsigned char ch = *((unsigned char*)addr + 2*1024*1024);
    ASSERT_TRUE(ch == 0xab);

    r = shm_remove(shmid);
    ASSERT_TRUE(r == 0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
