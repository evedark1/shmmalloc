#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shm_logger.h"
#include "shm_lock.h"

int shm_lock_init(pthread_mutex_t *mutex)
{
	pthread_mutexattr_t mat;
	int result;

	if ((result=pthread_mutexattr_init(&mat)) != 0) {
		logError("file: "__FILE__", line: %d, "
			"call pthread_mutexattr_init fail, "
			"errno: %d, error info: %s",
			__LINE__, result, strerror(result));
		return result;
	}
	if ((result=pthread_mutexattr_setpshared(&mat,
			PTHREAD_PROCESS_SHARED)) != 0)
	{
		logError("file: "__FILE__", line: %d, "
			"call pthread_mutexattr_setpshared fail, "
			"errno: %d, error info: %s",
			__LINE__, result, strerror(result));
		return result;
	}
	if ((result=pthread_mutexattr_settype(&mat,
			PTHREAD_MUTEX_NORMAL)) != 0)
	{
		logError("file: "__FILE__", line: %d, "
			"call pthread_mutexattr_settype fail, "
			"errno: %d, error info: %s",
			__LINE__, result, strerror(result));
		return result;
	}
    if((result=pthread_mutex_init(mutex, &mat)) != 0)
    {
		logError("file: "__FILE__", line: %d, "
			"call pthread_mutex_init fail, "
			"errno: %d, error info: %s",
			__LINE__, result, strerror(result));
		return result;
	}
	pthread_mutexattr_destroy(&mat);
	return 0;
}

static int do_lock_file(int fd, int cmd, int type)
{
    struct flock lock;
    int result;

    memset(&lock, 0, sizeof(lock));
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    do
    {
        if ((result=fcntl(fd, cmd, &lock)) != 0)
            result = errno != 0 ? errno : ENOMEM;
    } while (result == EINTR);

    return result;
}

int shm_lock_file(const char *filename)
{
    int result;
    mode_t old_mast;

    old_mast = umask(0);
    int lock_fd = open(filename, O_WRONLY, 0666);
    umask(old_mast);
    if (lock_fd < 0) {
        result = errno != 0 ? errno : EPERM;
        logError("file: "__FILE__", line: %d, "
                "open filename: %s fail, "
                "errno: %d, error info: %s", __LINE__,
                filename, result, strerror(result));
        return -1;
    }

    if ((result=do_lock_file(lock_fd, F_SETLKW, F_WRLCK)) != 0) {
        close(lock_fd);
        logError("file: "__FILE__", line: %d, "
                "lock filename: %s fail, "
                "errno: %d, error info: %s", __LINE__,
                filename, result, strerror(result));
        return -1;
    }

    return lock_fd;
}

void shm_unlock_file(int lock_fd)
{
    close(lock_fd);
}
