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

The quickest and safest way of creating an ULID is using
`ulid_make_urandom()`:

```c
ulid_t u;
ulid_make_urandom (&u);
```

It is possible to customize the timstamp and method used to obtain the random
bytes of entropy using the `ulid_encode()` function, which takes a timestamp
and a function pointer to fill an `ulid_t` value:

```c
ulid_t u;
ulid_encode (&u, ulid_clock_monotonic (), ulid_entropy_rand, NULL);
```

In order to obtain a textual representation of the value, use `ulid_string()`:

```
char s[ULID_STRING_LENGTH + 1];  // +1 for the null terminator
ulid_string (&u, s);
```


### Encoding Functions

- `ulid_encode(dst, timestamp, rng, userdata)` encodes an ULID value with a
  given *timestamp*, filling the entropy bytes by invoking the *rng* function,
  which gets passed the *userdata* value. While this allows customizing
  completely how an ULID is encoded, it is recommended to use the shorthand
  functions listed below.

- `ulid_string(ulid, buffer[ULID_STRING_LENGTH+1])` serializes an *ulid*
  value to a null-terminated C string *buffer*.


### Shorthand Functions

- `ulid_encode_rand(dst, timestamp)` uses `ulid_entropy_rand`.

- `ulid_encode_const(dst, timestamp, value)` uses `ulid_entropy_const`.

- `ulid_make_rand(dst)` uses `ulid_clock_monotonic` and `ulid_entropy_rand`.

- `ulid_make_const(dst, value)` uses `ulid_clock_monotonic` and
  `ulid_entropy_const`.

- `ulid_make_urandom(dst)` uses `ulid_clock_monotonic` and `getrandom(2)` (on
  Linux), `arc4random(3)` (on OpenBSD), or directly reads `/dev/random`
  (on other systems).


### Utility Functions

- `ulid_equal(a, b)` checks whether two `ulid_t` values are the same.

- `ulid_compare(a, b)` compares two `ulid_t` values and returns an `int`
  value (-1, 0, 1) which can be used for sorting ULIDs.

- `ulid_copy(dst, src)` copies an `ulid_t` into another.


### Timestamp Functions

Unless stated otherwise, all timestamp functions have millisecond resolution.
It is stronly recommended to use `ulid_clock_monotonic()`.

- `ulid_time_epoch()` returns seconds from the Epoch, with seconds resolution.
  Uses `time(2)` under the hood.

- `ulid_time_cpu_used()` returns milliseconds of CPU time used by the current
  process. Uses `clock(2)` under the hood.

- `ulid_clock_realtime()` returns the wall clock time, with milliseconds
  resolution. Uses `clock_gettime(2)` and `CLOCK_REALTIME`, falling back
  to `ulid_time_epoch()` on failure.

- `ulid_clock_monotonic()` returns an ever-increasing time. Uses
  `clock_gettime(2)` and `CLOCK_MONOTONIC_RAW` (if available, otherwise
  `CLOCK_MONOTONIC`), falling back to `ulid_clock_realtime()` on failure.

- `ulid_clock_cpu_used()` returns the amount of CPU time used by the current
  process. Uses `clock_gettime(2)` and `CLOCK_PROCESS_CPUTIME_ID` if
  available, otherwise falls back to `ulid_time_cpu_used()`.


### Entropy Functions

Those are provided to customize how the 10 entropy bytes of ULIDs are
filled-in. Most of the time these do not need to be used, and in general
it is strongly recommended to use the `ulid_encode_urandom()` or
`ulid_make_urandom()` shorthand functions instead.

- `ulid_entropy_const()` fills the entropy with the lower 8 bits of the
  constant value passed as *userdata*.

- `ulid_entropy_rand()` uses the `rand(3)` function from the standard C
  library. You might want to use `srand()` during the initialization of
  your program.

- `ulid_entropy_fd()` reads bytes from the Unix file descriptor passed
  as *userdata*, which is interpreted as an `int`.

- `ulid_entropy_file()` reads bytes from the `FILE*` handle passed as
  *userdata*.
