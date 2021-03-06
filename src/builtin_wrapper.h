#ifndef _BUILTIN_WRAPPER_H_
#define _BUILTIN_WRAPPER_H_
#include <limits.h>

#if (__WORDSIZE == 64)
static inline unsigned ffs_u64(unsigned long bit) {
    return __builtin_ffsl(bit);
}
#else
static inline unsigned ffs_u64(unsigned long long bit) {
    return __builtin_ffsll(bit);
}
#endif

static inline unsigned atomic_add_and_fetch(unsigned *ptr, unsigned val) {
    return __sync_add_and_fetch(ptr, val);
}

static inline unsigned atomic_sub_and_fetch(unsigned *ptr, unsigned val) {
    return __sync_sub_and_fetch(ptr, val);
}

#endif
