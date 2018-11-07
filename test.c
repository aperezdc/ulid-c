/*
 * test.c
 * Copyright (C) 2018 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#undef _NDEBUG
#define _DEBUG 1

#include "ulid.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int
main (int argc, char *argv[])
{
    srand (getpid ());

    /* Print a random ULID. */
    ulid_t u;
    ulid_make_urandom (&u);

    char s[ULID_STRING_LENGTH + 1];
    ulid_string (&u, s);
    printf ("%s\n", s);

    /* Sanity checks. */
    const uint64_t timestamp = ulid_clock_monotonic ();
    ulid_encode_const (&u, timestamp, 0xAB);
    assert(ulid_timestamp (&u) == timestamp);

    return EXIT_SUCCESS;
}
