/*
 * ulid.c
 * Copyright (C) 2018 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#define _POSIX_C_SOURCE 199309L

#if defined(__linux__)
# include <sys/random.h>
# define URANDOM_USE_GETRANDOM 1
#elif defined(__OpenBSD__)
# define _BSD_SOURCE
# include <stdlib.h>
# define URANDOM_USE_ARC4RANDOM 1
#endif

#include "deps/apicheck/apicheck.h"
#include "ulid.h"

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ULID_RNG_SKIP ((ulid_entropy_func_t) (-1))
#define ULID_TIMESTAMP_BYTES  6
#define ULID_ENTROPY_BYTES   10


uint64_t
ulid_time_epoch (void)
{
    return (uint64_t) time (NULL) * 1000;
}


uint64_t
ulid_time_cpu_used (void)
{
    /* XXX: Should this use times(2)? */
    return (uint64_t) (clock() / (CLOCKS_PER_SEC / 1000));
}


static inline uint64_t
timespec_to_ms (const struct timespec* const ts)
{
    assert (ts != NULL);
    return (uint64_t) (ts->tv_sec * 1000) + (uint64_t) (ts->tv_nsec / (1000 * 1000));
}


uint64_t
ulid_clock_realtime (void)
{
    struct timespec ts;

    if (clock_gettime (CLOCK_REALTIME, &ts) == 0)
        return timespec_to_ms (&ts);

    return ulid_time_epoch ();
}


#if defined(__linux__) && defined(CLOCK_MONOTONIC_RAW)
# define ULID_CLOCK_MONOTONIC CLOCK_MONOTONIC_RAW
#else
# define ULID_CLOCK_MONOTONIC CLOCK_MONOTONIC
#endif

uint64_t
ulid_clock_monotonic (void)
{
    struct timespec ts;

    if (clock_gettime (ULID_CLOCK_MONOTONIC, &ts) == 0)
        return timespec_to_ms (&ts);

    return ulid_clock_realtime ();
}


uint64_t
ulid_clock_cpu_used (void)
{
#if defined(CLOCK_PROCESS_CPUTIME_ID)
    struct timespec ts;

    if (clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &ts) == 0)
        return timespec_to_ms (&ts);
#endif

    return ulid_time_cpu_used ();
}


uint8_t
ulid_entropy_const (void* userdata)
{
    return (uint8_t) ((uintptr_t) userdata);
}

uint8_t
ulid_entropy_rand (void* userdata)
{
    return (uint8_t) rand ();
}

uint8_t
ulid_entropy_fd (void* userdata)
{
    uint8_t byte = 0;
    read ((int) ((intptr_t) userdata), &byte, 1);
    return byte;
}

uint8_t
ulid_entropy_file (void* userdata)
{
    FILE* filep = userdata;
    api_check_return_val (filep != NULL, 0);
    return fgetc (filep);
}


_Bool
ulid_equal (const ulid_t* const a,
            const ulid_t* const b)
{
    api_check_return_val (a != NULL, false);
    api_check_return_val (b != NULL, false);

    return a == b || memcmp (a->data, b->data, ULID_BYTES) == 0;
}


int
ulid_compare (const ulid_t* const a,
              const ulid_t* const b)
{
    api_check_return_val (a != NULL, false);
    api_check_return_val (b != NULL, false);

    return (a == b) ? 0 : memcmp (a->data, b->data, ULID_BYTES);
}


void
ulid_copy (ulid_t* dst, const ulid_t* const src)
{
    api_check_return (src != NULL);
    api_check_return (dst != NULL);
    memcpy (dst->data, src->data, ULID_BYTES);
}


extern void
ulid_encode  (ulid_t*             dst,
              uint64_t            timestamp,
              ulid_entropy_func_t rng,
              void*               userdata)
{
    api_check_return (dst != NULL);
    api_check_return (rng != NULL);

    /* Encode the lower 6 bytes of the time in the first six bytes. */
    dst->data[0] = (uint8_t) (timestamp >> 40);
    dst->data[1] = (uint8_t) (timestamp >> 32);
    dst->data[2] = (uint8_t) (timestamp >> 24);
    dst->data[3] = (uint8_t) (timestamp >> 16);
    dst->data[4] = (uint8_t) (timestamp >>  8);
    dst->data[5] = (uint8_t) (timestamp >>  0);

    /* Fill in the remaining 10 bytes of entropy. */
    if (rng != ULID_RNG_SKIP) {
        for (unsigned i = 6; i < ULID_BYTES; i++)
            dst->data[i] = (*rng) (userdata);
    }
}


enum {
    BIT0 = 1 << 0,
    BIT1 = 1 << 1,
    BIT2 = 1 << 2,
    BIT3 = 1 << 3,
    BIT4 = 1 << 4,
    BIT5 = 1 << 5,
    BIT6 = 1 << 6,
    BIT7 = 1 << 7,

    BITS_H0_L0 = (BIT0),
    BITS_H1_L0 = (BIT1 | BIT0),
    BITS_H2_L0 = (BIT2 | BIT1 | BIT0),
    BITS_H3_L0 = (BIT3 | BIT2 | BIT1 | BIT0),
    BITS_H4_L0 = (BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
    BITS_H5_L1 = (BIT5 | BIT4 | BIT3 | BIT2 | BIT1),
    BITS_H5_L3 = (BIT5 | BIT4 | BIT3),
    BITS_H6_L2 = (BIT6 | BIT5 | BIT4 | BIT3 | BIT2),
    BITS_H7_L3 = (BIT7 | BIT6 | BIT5 | BIT4 | BIT3),
    BITS_H7_L4 = (BIT7 | BIT6 | BIT5 | BIT4),
    BITS_H7_L5 = (BIT7 | BIT6 | BIT5),
    BITS_H7_L6 = (BIT7 | BIT6),
    BITS_H7_L7 = (BIT7),
};

void
ulid_string (const ulid_t* const ulid,
             char                buffer[ULID_STRINGZ_LENGTH])
{
    api_check_return (ulid != NULL);

#if !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
    api_check_return (buffer != NULL);
#endif

    static const char B32[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

#define BITx(srcidx, hb, lb) \
    (ulid->data[(srcidx)] & (BITS_H ## hb ## _L ## lb))

#define BITr(srcidx, hb, lb) \
    (BITx (srcidx, hb, lb) >> (lb))

#define BITl(srcidx, hb, lb) \
    (BITx (srcidx, hb, lb) << (5 - (hb) - (lb) - 1))

#define BB(dstidx, va, vb) \
    buffer[(dstidx)] = B32[(va) | (vb)]

    /* Timestamp: data[0..5] */
    BB ( 0, 0,               BITr ( 0, 7, 5)); /* data[0]<7:5>              */
    BB ( 1, 0,               BITr ( 0, 4, 0)); /* data[0]<4:0>              */
    BB ( 2, 0,               BITr ( 1, 7, 3)); /* data[1]<7:3>              */
    BB ( 3, BITl ( 1, 2, 0), BITr ( 2, 7, 6)); /* data[1]<2:0> data[2]<7:6> */
    BB ( 4, 0,               BITr ( 2, 5, 1)); /* data[2]<5:1>              */
    BB ( 5, BITl ( 2, 0, 0), BITr ( 3, 7, 4)); /* data[2]<0:0> data[3]<7:4> */
    BB ( 6, BITl ( 3, 3, 0), BITr ( 4, 7, 7)); /* data[3]<3:0> data[4]<7:7> */
    BB ( 7, 0,               BITr ( 4, 6, 2)); /* data[4]<6:2>              */
    BB ( 8, BITl ( 4, 1, 0), BITr ( 5, 7, 5)); /* data[4]<1:0> data[5]<7:5> */
    BB ( 9, 0,               BITr ( 5, 4, 0)); /* data[5]<4:0>              */

    /* Entropy: data[6..15] */
    BB (10, 0,               BITr ( 6, 7, 3)); /* data[6]<7:3>              */
    BB (11, BITl ( 6, 2, 0), BITr ( 7, 7, 6)); /* data[6]<2:0> data[7]<7:6> */
    BB (12, 0,               BITr ( 7, 5, 1)); /* data[7]<5:1>              */
    BB (13, BITl ( 7, 0, 0), BITr ( 8, 7, 4)); /* data[7]<0:0> data[8]<7:4> */
    BB (14, BITl ( 8, 3, 0), BITr ( 9, 7, 7)); /* data[8]<3:0> data[9]<7:7> */
    BB (15, 0,               BITr ( 9, 6, 2)); /* data[9]<6:2>              */
    BB (16, BITl ( 9, 1, 0), BITr (10, 7, 5)); /* data[9]<1:0> data[A]<7:5> */
    BB (17, 0,               BITr (10, 4, 0)); /* data[A]<4:0>              */

    BB (18, 0,               BITr (11, 7, 3)); /* data[B]<7:3>              */
    BB (19, BITl (11, 2, 0), BITr (12, 7, 6)); /* data[B]<2:0> data[C]<7:6> */
    BB (20, 0,               BITr (12, 5, 1)); /* data[C]<5:1>              */
    BB (21, BITl (12, 0, 0), BITr (13, 7, 4)); /* data[C]<0:0> data[D]<7:4> */
    BB (22, BITl (13, 3, 0), BITr (14, 7, 7)); /* data[D]<3:0> data[E]<7:7> */
    BB (23, 0,               BITr (14, 6, 2)); /* data[E]<6:2>              */
    BB (24, BITl (14, 1, 0), BITr (15, 7, 5)); /* data[E]<1:0> data[F]<7:5> */
    BB (25, 0,               BITr (15, 4, 0)); /* data[F]<4:0>              */

#undef BITr
#undef BITl
#undef BITx

    /* Null-terminate the string. */
    buffer[ULID_STRING_LENGTH] = '\0';
}


uint64_t
ulid_timestamp (const ulid_t* const ulid)
{
    api_check_return_val (ulid != NULL, 0);

    return (uint64_t) ulid->data[0] << 40
         | (uint64_t) ulid->data[1] << 32
         | (uint64_t) ulid->data[2] << 24
         | (uint64_t) ulid->data[3] << 16
         | (uint64_t) ulid->data[4] <<  8
         | (uint64_t) ulid->data[5] <<  0
         ;
}

void
ulid_encode_urandom (ulid_t *dst,
                     uint64_t timestamp)
{
    ulid_encode (dst, timestamp, ULID_RNG_SKIP, NULL);

#if defined(URANDOM_USE_GETRANDOM)
    if (getrandom (&dst->data[ULID_TIMESTAMP_BYTES], ULID_ENTROPY_BYTES, 0) == ULID_ENTROPY_BYTES)
        return;
#elif defined(URANDOM_USE_ARC4RANDOM)
    arc4random_buf (&dst->data[ULID_TIMESTAMP_BYTES], ULID_ENTROPY_BYTES);
    return;
#endif

    /* As a fallback implementation, try reading from /dev/urandom. */
    int fd = open ("/dev/urandom", O_RDONLY);
    if (fd == -1)
        goto fallback;

    size_t nread = read (fd,
                         &dst->data[ULID_TIMESTAMP_BYTES],
                         ULID_ENTROPY_BYTES);
    close (fd);
    if (nread == ULID_ENTROPY_BYTES)
        return;

fallback:
    ulid_encode (dst, timestamp, ulid_entropy_rand, NULL);
}
