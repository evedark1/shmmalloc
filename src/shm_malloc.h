/* shm malloc interface
 */
#ifndef _SHM_MALLOC_H_
#define _SHM_MALLOC_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// SHM_NULL stand for NULL shm malloc pos
#define SHM_NULL 0

/*
init shm malloc
parameters:
    path: path identify share memory, used by ftok
    create: create != 0, create share memory if not exist
return 0 for success, else retrun errno
*/
extern int shm_init(const char *path, int create);

/*
shm malloc, like malloc
parameters:
    size: malloc size
return malloc position, 0 for fail
*/
extern uint64_t shm_malloc(size_t size);

/*
shm free, like free
parameters:
    pos: free position
*/
extern void shm_free(uint64_t pos);

/*
shm realloc, like realloc
parameters:
    pos: change position, if pos == SHM_NULL realloc equal to malloc
    size: change to size
return malloc pos, 0 for fail
*/
extern uint64_t shm_realloc(uint64_t pos, size_t size);

/*
shm check size, return avaliable size
parameters:
    pos: check position
return malloc pos, 0 for fail
*/
extern size_t shm_check_size(uint64_t pos);

/*
shm set user data, user data is shared between diff process
parameters:
    data: user data to set
*/
extern void shm_set_userdata(uint64_t data);

/*
shm get user data, user data is shared between diff process
return user data, default 0
*/
extern uint64_t shm_get_userdata();

/*
get addr pointer from shm position, addr is valid until free
share memory may free by other process
parameters:
    pos: find position
return addr, NULL for fail
*/
extern void *shm_get_addr(uint64_t pos);

/*
set log handle, default use printf to stdout
parameters:
    level: output log level, defined in syslog.h like LOG_ERR
    str: output log info
*/
typedef void (*shm_log_handle)(int level, const char *str);
extern void shm_set_log_callback(shm_log_handle cb);

/*
set log level, default LOG_WARNING
parameters:
    level: level is defined in syslog.h, like LOG_ERR
*/
extern void shm_set_log_level(int level);

/*
get log level, default LOG_WARNING
return current log level
*/
extern int shm_get_log_level();

#ifdef __cplusplus
}
#endif

#endif
