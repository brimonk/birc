#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

typedef int8_t     i8;
typedef int16_t    i16;
typedef int32_t    i32;
typedef int64_t    i64;
typedef uint8_t    u8;
typedef uint16_t   u16;
typedef uint32_t   u32;
typedef uint64_t   u64;
typedef float      f32;
typedef double     f64;

#define BUFSMALL  (1 << 10)
#define BUFLARGE  (1 << 16)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARRSIZE(x) (sizeof((x))/sizeof((x)[0]))
#define SWAP(a, b) do { typeof((a)) z_ = (b); (b) = (a); (a) = z_; } while (0);

#define streq(a, b) (strcmp((a), (b)) == 0 && strlen((a)) == strlen((b)))

#ifdef COMMON_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION // useful for later :)

char *bfgets(char *s, size_t n, FILE *fp)
{
    char *t = fgets(s, n, fp);
    if (t != NULL) {
        s[strlen(s) - 1] = 0;
    }
    return t;
}

char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s)
{
    for (char *e = s + strlen(s) - 1; isspace(*e); e--)
        *e = 0;
    return s;
}

char *trim(char *s)
{
    return ltrim(rtrim(s));
}

// COMPARATORS

int comp_i32(const void *a, const void *b)
{
	i32 ia = *(i32 *)a;
	i32 ib = *(i32 *)b;
	if (ia > ib) return -1;
	if (ia < ib) return 1;
	return 0;
}

#endif // COMMON_IMPLEMENTATION

#include "stb_ds.h"

#endif // COMMON_H_
