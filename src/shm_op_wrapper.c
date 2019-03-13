#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "shm_logger.h"
#include "shm_op_wrapper.h"

void *shm_do_shmmap(key_t key, size_t size, bool create_segment, int *err_no)
{
    int shmid;
    void *addr = NULL;

    if (create_segment) {
        shmid = shmget(key, size, IPC_CREAT | 0666);
    } else {
        shmid = shmget(key, 0, 0666);
    }
    if (shmid < 0) {
        *err_no = errno != 0 ? errno : EPERM;
        logError("file: "__FILE__", line: %d, "
                "shmget with key %08x fail, "
                "errno: %d, error info: %s", __LINE__,
                key, *err_no, strerror(*err_no));
        return NULL;
    }

    addr = shmat(shmid, NULL, 0);
    if (addr == NULL || addr == (void *)-1) {
        *err_no = errno != 0 ? errno : EPERM;
        logError("file: "__FILE__", line: %d, "
                "shmat with key %08x fail, "
                "errno: %d, error info: %s", __LINE__,
                key, *err_no, strerror(*err_no));
        return NULL;
    }

    *err_no = 0;
    return addr;
}

static int write_file(const char *filename, const char *buff, int file_size)
{
    int fd;
    int result;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        result = errno != 0 ? errno : EIO;
        logError("file: "__FILE__", line: %d, "
            "open file %s fail, errno: %d",
            __LINE__, filename, result);
        return result;
    }

    if (write(fd, buff, file_size) != file_size)
    {
        result = errno != 0 ? errno : EIO;
        logError("file: "__FILE__", line: %d, "
            "write file %s fail, errno: %d",
            __LINE__, filename, result);
        close(fd);
        return result;
    }

    if (fsync(fd) != 0)
    {
        result = errno != 0 ? errno : EIO;
        logError("file: "__FILE__", line: %d, "
            "fsync file \"%s\" fail, errno: %d",
            __LINE__, filename, result);
        close(fd);
        return result;
    }

    close(fd);
    return 0;
}

static int shm_get_key(const char *filename, int proj_id,
        key_t *key)
{
    int result;
    if (access(filename, F_OK) != 0) {
        result = errno != 0 ? errno : ENOENT;
        if (result != ENOENT) {
            logError("file: "__FILE__", line: %d, "
                    "access filename: %s fail, "
                    "errno: %d, error info: %s", __LINE__,
                    filename, result, strerror(result));
            return result;
        }

        result = write_file(filename, "FOR LOCK", 8);
        if (result != 0) {
            return result;
        }
        if (chmod(filename, 0666) != 0) {
            result = errno != 0 ? errno : EFAULT;
            logError("file: "__FILE__", line: %d, "
                    "chmod filename: %s fail, "
                    "errno: %d, error info: %s", __LINE__,
                    filename, result, strerror(result));
            return result;
        }
    }

    *key = ftok(filename, proj_id);
    if (*key == -1) {
        result = errno != 0 ? errno : EFAULT;
        logError("file: "__FILE__", line: %d, "
                "call fc_ftok fail, filename: %s, proj_id: %d, "
                "errno: %d, error info: %s", __LINE__,
                filename, proj_id, result, strerror(result));
        return result;
    }
    return 0;
}

void *shm_mmap(const char *filename,
        int proj_id, size_t size, key_t *key,
        bool create_segment, int *err_no)
{
    if ((*err_no=shm_get_key(filename, proj_id, key)) != 0) {
        return NULL;
    }
    return shm_do_shmmap(*key, size, create_segment, err_no);
}

int shm_munmap(void *addr, size_t size)
{
    int result;
    if (shmdt(addr) == 0) {
        result = 0;
    } else {
        result = errno != 0 ? errno : EACCES;
        logError("file: "__FILE__", line: %d, "
                "munmap addr: %p, size: %"PRId64" fail, "
                "errno: %d, error info: %s", __LINE__,
                addr, size, errno, strerror(errno));
    }
    return result;
}

int shm_remove(key_t key)
{
    int result;
    int shmid;

    shmid = shmget(key, 0, 0666);
    if (shmid < 0) {
        result = errno != 0 ? errno : EACCES;
        logError("file: "__FILE__", line: %d, "
                "shmget with key %08x fail, "
                "errno: %d, error info: %s", __LINE__,
                key, errno, strerror(errno));
        return result;
    }

     if (shmctl(shmid, IPC_RMID, NULL) != 0) {
        result = errno != 0 ? errno : EACCES;
        logError("file: "__FILE__", line: %d, "
                "remove shm with key %08x fail, "
                "errno: %d, error info: %s", __LINE__,
                key, errno, strerror(errno));
        return result;
     }
    return 0;
}

bool shm_exists(const char *filename, int proj_id)
{
    key_t key;
    if (shm_get_key(filename, proj_id, &key) != 0) {
        return false;
    }
    return shmget(key, 0, 0666) >= 0;
}
