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
    ulid_t u, v;
    ulid_make_urandom (&u);

    assert (ulid_equal (&u, &u));

    ulid_copy (&v, &u);
    assert (ulid_equal (&u, &v));

    char s[ULID_STRINGZ_LENGTH];
    ulid_string (&u, s);
    printf ("%s\n", s);

    /* Roundtrip. */
    _Bool parsed_ok = ulid_parse (&v, s);
    assert (parsed_ok);
    assert (ulid_equal (&u, &v));

    /* Sanity checks. */
    const uint64_t timestamp = ulid_clock_monotonic ();
    ulid_encode_const (&u, timestamp, 0xAB);
    assert(ulid_timestamp (&u) == timestamp);

    const uint64_t newer_timestamp = ulid_clock_monotonic ();
    ulid_encode_const (&v, newer_timestamp, 0xCD);
    assert (ulid_timestamp (&v) == newer_timestamp);

    assert (!ulid_equal (&u, &v));
    assert (ulid_compare (&u, &v) < 0);

    return EXIT_SUCCESS;
}
