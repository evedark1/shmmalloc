#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shm_malloc.h"

#define SMALL_TEST_SIZE 1024
#define MEDIUM_TEST_SIZE 1024
#define LARGE_TEST_SIZE 4
struct memory_info {
    uint64_t small[SMALL_TEST_SIZE];
    uint64_t medium[MEDIUM_TEST_SIZE];
    uint64_t large[LARGE_TEST_SIZE];
};

void check_exit(bool r, char *msg) {
    if(r) {
        printf("%s\n", msg);
        exit(0);
    }
}

uint64_t malloc_test() {
    uint64_t info_pos = shm_malloc(sizeof(struct memory_info));
    check_exit(info_pos == SHM_NULL, "shm malloc error\n");
    struct memory_info *info = shm_get_addr(info_pos);
    printf("shm malloc %lx %p\n", info_pos, info);

    for(int i = 0; i < SMALL_TEST_SIZE; i++) {
        uint64_t p;
        if(i < 256) {
            p = shm_malloc(8);
        } else {
            p = shm_malloc(64 * (i / 32));
        }
        check_exit(p == SHM_NULL, "shm malloc small error");
        info->small[i] = p;
        uint64_t *a = shm_get_addr(p);
        *a = i;
    }

    for(int i = 0; i < MEDIUM_TEST_SIZE; i++) {
        uint64_t p = shm_malloc((i / 64 + 1) * 4096);
        check_exit(p == SHM_NULL, "shm malloc medium error");
        info->medium[i] = p;
        uint64_t *a = shm_get_addr(p);
        *a = i;
    }

    for(int i = 0; i < LARGE_TEST_SIZE; i++) {
        uint64_t p = shm_malloc((i + 1) * 1024 * 1024);
        check_exit(p == SHM_NULL, "shm malloc large error");
        info->large[i] = p;
        uint64_t *a = shm_get_addr(p);
        *a = i;
    }
    return info_pos;
}

void free_test(uint64_t info_pos) {
    struct memory_info *info = shm_get_addr(info_pos);
    printf("shm free %lx %p\n", info_pos, info);

    for(int i = 0; i < SMALL_TEST_SIZE; i++) {
        shm_free(info->small[i]);
    }
    for(int i = 0; i < MEDIUM_TEST_SIZE; i++) {
        shm_free(info->medium[i]);
    }
    for(int i = 0; i < LARGE_TEST_SIZE; i++) {
        shm_free(info->large[i]);
    }
    shm_free(info_pos);
}

void read_test(uint64_t info_pos) {
    struct memory_info *info = shm_get_addr(info_pos);
    check_exit(info == NULL, "shm reader get addr error\n");
    printf("shm read %lx %p\n", info_pos, info);

    uint64_t i;
    for(i = 0; i < SMALL_TEST_SIZE; i++) {
        uint64_t *p = shm_get_addr(info->small[i]);
        check_exit(p == NULL, "shm read error");
        if(*p != i) {
            printf("read %ld error\n", i);
            break;
        }
    }
    if(i == 2048) {
        printf("read shm success\n");
    }
}

int main() {
    int r = 0;
    shm_set_log_level(LOG_INFO);
    if((r = shm_init("/tmp/shm_example", 1)) != 0) {
        printf("init shm error %d\n", r);
        return 0;
    }

    uint64_t data = shm_get_userdata();
    if(data == 0) {
        uint64_t pos = malloc_test();
        shm_set_userdata(pos);

        // wait end
        getchar();
        free_test(pos);
        shm_set_userdata(0);
    } else {
        read_test(data);
        // wait end
        getchar();
    }

    return 0;
}
