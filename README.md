shmmalloc 是一个基于 System V 共享内存实现的内存分配器，包含基本的malloc/free函数和其他附加函数。

### 编译

我在64位Ubuntu 16.04上进行编译，其他环境没有进行测试。下面命令可以编译生成静态库。
```shell
mkdir build
cd build
cmake ..
make shmmalloc
```

### 使用实例

```c
#include <stdio.h>
#include <string.h>
#include "shm_malloc.h"

int main() {
    // init use /tmp/shm_demo path
    if(shm_init("/tmp/shm_demo", 1) != 0) {
        printf("init shm error\n");
        return 0;
    }

    // malloc shared memory
    uint64_t pos = shm_malloc(32);
    char *c = shm_get_addr(pos);
    strcpy(c, "shm malloc test");
    printf("shared string: %s\n", c);
    
    // free shared memory
    shm_free(pos);
    return 0;
}
```