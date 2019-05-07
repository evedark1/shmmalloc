#ifndef _SHM_LOGGER_H_
#define _SHM_LOGGER_H_

#include <syslog.h>
#include <sys/time.h>

/* use local log func */
extern void shm_log(int level, const char *format, ...);

#define logEmerg(fmt, args...) shm_log(LOG_EMERG, fmt, ##args)
#define logAlert(fmt, args...) shm_log(LOG_ALERT, fmt, ##args)
#define logCrit(fmt, args...) shm_log(LOG_CRIT, fmt, ##args)
#define logError(fmt, args...) shm_log(LOG_ERR, fmt, ##args)
#define logWarning(fmt, args...) shm_log(LOG_WARNING, fmt, ##args)
#define logNotice(fmt, args...) shm_log(LOG_NOTICE, fmt, ##args)
#define logInfo(fmt, args...) shm_log(LOG_INFO, fmt, ##args)
#define logDebug(fmt, args...) shm_log(LOG_DEBUG, fmt, ##args)

#endif
