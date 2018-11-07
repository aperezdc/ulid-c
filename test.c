/*
 * test.c
 * Copyright (C) 2018 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "ulid.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int
main (int argc, char *argv[])
{
    srand (getpid ());

    ulid_t u;
    ulid_make_urandom (&u);
    // ulid_encode (&u, ~0, ulid_entropy_rand, NULL);

    char s[ULID_STRING_LENGTH + 1];
    ulid_string (&u, s);
    printf ("%s\n", s);

    return EXIT_SUCCESS;
}
