/*
 * ulid.h
 * Copyright (C) 2018 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef ULID_H
#define ULID_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
# ifndef __TINYC__
#  define ULID_STATIC_SIZE(v) static ULID_ ## v ## _LENGTH
# endif
#else
# define inline __inline
#endif

#ifndef ULID_STATIC_SIZE
#define ULID_STATIC_SIZE(v) ULID_ ## v ## _LENGTH
#endif /* !ULID_STATIC_SIZE */

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cpluspls
extern "C" {
#endif

enum {
    ULID_BITS = 128,
    ULID_BYTES = ULID_BITS / 8,
    ULID_STRING_LENGTH = 26,
    ULID_STRINGZ_LENGTH = ULID_STRING_LENGTH + 1,
};

typedef struct ulid ulid_t;

struct ulid {
    uint8_t data[ULID_BYTES];
};

typedef uint8_t (*ulid_entropy_func_t) (void*);

extern uint64_t ulid_time_epoch      (void);
extern uint64_t ulid_time_cpu_used   (void);
extern uint64_t ulid_clock_realtime  (void);
extern uint64_t ulid_clock_monotonic (void);
extern uint64_t ulid_clock_cpu_used  (void);

extern uint8_t  ulid_entropy_const   (void*);
extern uint8_t  ulid_entropy_rand    (void*);
extern uint8_t  ulid_entropy_fd      (void*);
extern uint8_t  ulid_entropy_file    (void*);

extern _Bool ulid_equal   (const ulid_t* const a,
                           const ulid_t* const b);

extern int   ulid_compare (const ulid_t* const a,
                           const ulid_t* const b);

extern void  ulid_copy    (ulid_t*             dst,
                           const ulid_t* const src);

extern void  ulid_string  (const ulid_t* const ulid,
                           char                buffer[ULID_STATIC_SIZE(STRINGZ)]);

extern void  ulid_encode  (ulid_t*             dst,
                           uint64_t            timestamp,
                           ulid_entropy_func_t rng,
                           void*               userdata);

extern uint64_t ulid_timestamp (const ulid_t* const ulid);

extern void  ulid_encode_urandom (ulid_t *dst,
                                  uint64_t timestamp);

static inline void
ulid_encode_rand (ulid_t* dst, uint64_t timestamp)
{
    ulid_encode (dst, timestamp, ulid_entropy_rand, (void*) 0);
}

static inline void
ulid_encode_const (ulid_t* dst, uint64_t timestamp, uint8_t value)
{
    ulid_encode (dst, timestamp, ulid_entropy_const, (void*) ((uintptr_t) value));
}

static inline void
ulid_make_rand (ulid_t *dst)
{
    ulid_encode_rand (dst, ulid_clock_monotonic ());
}

static inline void
ulid_make_const (ulid_t* dst, uint8_t value)
{
    ulid_encode_const (dst, ulid_clock_monotonic (), value);
}

static inline void
ulid_make_urandom (ulid_t* dst)
{
    ulid_encode_urandom (dst, ulid_clock_monotonic ());
}

#ifdef __cplusplus
}
#endif

#endif /* !ULID_H */
