#ifndef _BUILTIN_WRAPPER_H
#define _BUILTIN_WRAPPER_H
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (__WORDSIZE == 64)
static inline unsigned ffs_u64(unsigned long bit) {
    return __builtin_ffsl(bit);
}
#else
static inline unsigned ffs_u64(unsigned long long bit) {
    return __builtin_ffsll(bit);
}
#endif


#ifdef __cplusplus
}
#endif

#endif

