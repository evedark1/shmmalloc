#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shm_malloc.h"

void check_exit(bool r, char *msg) {
    if(r) {
        printf("%s\n", msg);
        exit(0);
    }
}

uint64_t malloc_test() {
    uint64_t arrpos = shm_malloc(2 * 1024 * 1024);
    check_exit(arrpos == SHM_NULL, "shm malloc error\n");
    uint64_t *addr = shm_get_addr(arrpos);
    printf("shm malloc %lx %p\n", arrpos, addr);

    for(int i = 0; i < 2048; i++) {
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
        void *addr = shm_get_addr(data);
        if(addr == NULL) {
            printf("shm reader get addr error\n");
            return 0;
        }
        printf("shm read: %p %u\n", addr, *(uint32_t*)addr);
        // wait end
        getchar();
    }

    return 0;
}
