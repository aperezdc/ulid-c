ulid-c
======

Standalone, plain C, clean-room implementation of
[ULID](https://github.com/ulid/spec).

Installation
------------

Using [clib](https://github.com/clibs/clib):

```sh
clib install aperezdc/ulid-c --save
```

Usage
-----

The `ulid_encode()` function takes a timestamp and a function that generates
random entropy, and fills in an `ulid_t` value:

```c
ulid_t u;
ulid_encode (&u, ulid_clock_monotonic (), ulid_entropy_rand, NULL);
```

In order to obtain a textual representation of the value, use `ulid_string()`:

```
char s[ULID_STRING_LENGTH + 1];  // +1 for the null terminator
ulid_string (&u, s);
```

