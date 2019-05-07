#ifndef _SHM_OP_WRAPPER_H_
#define _SHM_OP_WRAPPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

/**
shmget & shmat
parameters:
    key: shm key, if key <= 0, use id as shmid input
    size: share memory size
    create_segment: if create segment when segment not exist
    id: shmid input and output
    err_no: return errno
return share memory pointer, NULL for fail
*/
extern void *shm_mmap(key_t key, size_t size, bool create_segment, int *id, int *err_no);

/**
get shm key to use
parameters:
    filename: the filename
    proj_id: the project id to generate key
    key: return the key
return: errno, 0 for success, != 0 fail
*/
extern int shm_get_key(const char *filename, int proj_id, key_t *key);

/**
shmdt
parameters:
    addr: the address to munmap
return: errno, 0 for success, != 0 fail
*/
extern int shm_munmap(void *addr);

/**
remove shm, set IPC_RMID flag
parameters:
    shmid: shmid to change
return: errno, 0 for success, != 0 fail
*/
extern int shm_remove(int shmid);

#endif
