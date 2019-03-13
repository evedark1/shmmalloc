#include "shm_logger.h"
#include "shm_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

static shm_log_handle shm_log_callback = NULL;
static int shm_log_level = LOG_WARNING;
static char *level_str[8] = {
    "emerg", "alert", "crit",
    "err", "warning", "notice",
    "info", "debug"
};

void shm_log(int level, const char *format, ...)
{
    if(level > shm_log_level)
        return;

    char buf[LINE_MAX];
    va_list	param;
    va_start(param, format);
    vsnprintf(buf, LINE_MAX, format, param);
    va_end(param);

    if(shm_log_callback == NULL)
    {
        printf("[%s]: %s\n", level_str[level], buf);
    }
    else
    {
        shm_log_callback(level, buf);
    }
}

void shm_set_log_callback(shm_log_handle cb)
{
    shm_log_callback = cb;
}

int shm_get_log_level()
{
    return shm_log_level;
}

void shm_set_log_level(int level)
{
    shm_log_level = level;
}
