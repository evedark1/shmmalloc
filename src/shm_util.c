#include "shm_util.h"
#include <stdlib.h>
#include <string.h>

char *copystr(const char *src)
{
    char *dst = malloc(strlen(src) + 1);
    if(dst == NULL)
        return NULL;
    strcpy(dst, src);
    return dst;
}
