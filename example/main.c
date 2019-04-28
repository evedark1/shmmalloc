#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shm_malloc.h"

#define SMALL_TEST_SIZE 2048

void check_exit(bool r, char *msg) {
    if(r) {
        printf("%s\n", msg);
        exit(0);
    }
}

uint64_t malloc_test() {
    uint64_t arrpos = shm_malloc(SMALL_TEST_SIZE * sizeof(uint64_t));
    check_exit(arrpos == SHM_NULL, "shm malloc error\n");
    uint64_t *addr = shm_get_addr(arrpos);
    printf("shm malloc %lx %p\n", arrpos, addr);

    for(int i = 0; i < SMALL_TEST_SIZE; i++) {
        uint64_t p = shm_malloc(8);
        check_exit(p == SHM_NULL, "shm malloc small error");
        addr[i] = p;
        uint64_t *a = shm_get_addr(p);
        *a = i;
    }
    return arrpos;
}

void free_test(uint64_t arrpos) {
    uint64_t *addr = shm_get_addr(arrpos);
    printf("shm free %lx %p\n", arrpos, addr);

    for(int i = 0; i < 2048; i++) {
        shm_free(addr[i]);
    }
    shm_free(arrpos);
}

void read_test(uint64_t arrpos) {
    uint64_t *addr = shm_get_addr(arrpos);
    check_exit(addr == NULL, "shm reader get addr error\n");
    printf("shm read %lx %p\n", arrpos, addr);

    uint64_t i;
    for(i = 0; i < SMALL_TEST_SIZE; i++) {
        uint64_t *p = shm_get_addr(addr[i]);
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
