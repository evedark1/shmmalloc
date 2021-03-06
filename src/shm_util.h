#ifndef _SHM_UTIL_H_
#define _SHM_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "builtin_wrapper.h"

typedef uint64_t bitmap_t;
#define BITMAP_BITS 64
#define BITMAP_SHIFT 6	// 2^6 = 64
#define BITMAP_MASK (BITMAP_BITS - 1)
#define BITMAP_ONE (bitmap_t)1
/* Number of bitmap_t required to store a given number of bits. */
#define BITMAP_BITS2GROUPS(nbits) ((nbits + BITMAP_MASK) >> BITMAP_SHIFT)

static inline void bitmap_set(bitmap_t *bitmap, uint32_t n) {
    uint32_t word = n >> BITMAP_SHIFT;
    uint32_t position = n & BITMAP_MASK;
    bitmap[word] |= BITMAP_ONE << position;
}

static inline bool bitmap_get(const bitmap_t *bitmap, uint32_t n) {
    uint32_t word = n >> BITMAP_SHIFT;
    uint32_t position = n & BITMAP_MASK;
    return (bitmap[word] >> position) & BITMAP_ONE;
}

static inline void bitmap_clear(bitmap_t *bitmap, uint32_t n) {
    uint32_t word = n >> BITMAP_SHIFT;
    uint32_t position = n & BITMAP_MASK;
    bitmap[word] &= ~(BITMAP_ONE << position);
}

/*
find first bit unset
parameters:
    bitmap: find bitmap
    b: bit limit length
return index of first bit, = b not find
*/
static inline uint32_t bitmap_ffu(const bitmap_t *bitmap, uint32_t b) {
    uint32_t r = b;
    uint32_t l = BITMAP_BITS2GROUPS(b);
    for(uint32_t i = 0; i < l; i++) {
        bitmap_t g = ~bitmap[i];
        unsigned bit = ffs_u64(g);
        if (bit != 0) {
            r = (i << BITMAP_SHIFT) + (bit - 1);
            break;
        }
    }
    return r;
}

/*
find first bit set
parameters:
    bitmap: find bitmap
    b: bit limit length
return index of first bit, = b not find
*/
static inline uint32_t bitmap_ffs(const bitmap_t *bitmap, uint32_t b) {
    uint32_t r = b;
    uint32_t l = BITMAP_BITS2GROUPS(b);
    for(uint32_t i = 0; i < l; i++) {
        unsigned bit = ffs_u64(bitmap[i]);
        if (bit != 0) {
            r = (i << BITMAP_SHIFT) + (bit - 1);
            break;
        }
    }
    return r;
}

/*
find first bit set from position
parameters:
    bitmap: find bitmap
    n: bit start
    b: bit limit length
return index of first bit, = b not find
*/
static inline uint32_t bitmap_ffsp(const bitmap_t *bitmap, uint32_t n, uint32_t b) {
    assert(n < b);
    uint32_t r = b;
    uint32_t l = BITMAP_BITS2GROUPS(b);
    uint32_t i = n >> BITMAP_SHIFT;
    bitmap_t g = bitmap[i] & ~((1LU << (n & BITMAP_MASK)) - 1);
    do {
        unsigned bit = ffs_u64(g);
        if(bit != 0) {
            r = (i << BITMAP_SHIFT) + (bit - 1);
            break;
        }
        i++;
        g = bitmap[i];
    } while(i < l);
    return r;
}

/*
find and set first bit unset
parameters:
    bitmap: find bitmap
    b: bit limit length
return index of first bit, = b not find
*/
static inline uint32_t bitmap_sfu(bitmap_t *bitmap, uint32_t b) {
    uint32_t r = bitmap_ffu(bitmap, b);
    if(r < b)
        bitmap_set(bitmap, r);
    return r;
}

static inline size_t align_size(size_t size, size_t align) {
    return align * ((size + (align - 1)) / align);
}

// malloc and copy str
extern char *copystr(const char *src);

#endif

