#ifndef _SHM_LOCK_H
#define _SHM_LOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
init phtread_mutex with PTHREAD_PROCESS_SHARED
parameters:
    mutex: the mutex pointer
return errno, 0 for success, != 0 fail
*/
extern int shm_lock_init(pthread_mutex_t *mutex);

/**
lock exist file waiting
parameters:
    filename: exist file to lock
return >= 0 lock fd, < 0 fail
*/
extern int shm_lock_file(const char *filename);

/**
unlock lock fd
parameters:
    lock fd: shm_lock_file return
*/
extern void shm_unlock_file(int lock_fd);

#ifdef __cplusplus
}
#endif

#endif

