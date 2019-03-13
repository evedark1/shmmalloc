//shm_op_wrapper.h

#ifndef _SHM_OP_WRAPPER_H
#define _SHM_OP_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
shmget & shmat
parameters:
    key: shm key
    size: share memory size
    create_segment: if create segment when segment not exist
    err_no: return errno
return share memory pointer, NULL for fail
*/
extern void *shm_do_shmmap(key_t key, size_t size, bool create_segment, int *err_no);

/**
shmget & shmat
parameters:
	filename: the filename
	proj_id: the project id to generate key
    size: the share memory size
    key: return the key
    create_segment: if create segment when segment not exist
    err_no: return errno
return share memory pointer, NULL for fail
*/
extern void *shm_mmap(const char *filename,
        int proj_id, size_t size, key_t *key,
        bool create_segment, int *err_no);

/**
shmdt
parameters:
    addr: the address to munmap
    size: the share memory size
return: errno, 0 for success, != 0 fail
*/
extern int shm_munmap(void *addr, size_t size);

/**
remove shm
parameters:
    key: shm key
return: errno, 0 for success, != 0 fail
*/
extern int shm_remove(key_t key);

/**
if shm exists
parameters:
	filename: the filename
	proj_id: the project id to generate key
return: errno, 0 for success, != 0 fail
*/
extern bool shm_exists(const char *filename, const int proj_id);

#ifdef __cplusplus
}
#endif

#endif

