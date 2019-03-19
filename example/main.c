#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include "shm_malloc.h"

uint64_t large_malloc_test() {
    //uint64_t pos = shm_malloc(2 * 1024 * 1024);	// large malloc
    uint64_t pos = shm_malloc(8);	// small malloc
    if(pos == SHM_NULL) {
        printf("shm malloc error\n");
        return 0;
    }

    void *addr = shm_get_addr(pos);
    if(addr == NULL) {
        printf("shm writer get addr error\n");
        return 0;
    }
    strcpy(addr, "hello shm malloc, this is large malloc test\n");
    shm_set_userdata(pos);
    return pos;
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
        uint64_t pos = large_malloc_test();

        // wait end
        getchar();
        shm_free(pos);
        shm_set_userdata(0);
    } else {
        void *addr = shm_get_addr(data);
        if(addr == NULL) {
            printf("shm reader get addr error\n");
            return 0;
        }
        printf("shm read: %s\n", (char*)addr);
        // wait end
        getchar();
    }

    return 0;
}
