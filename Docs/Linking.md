# Linking `libbrf.a` Into Your Program

## A Linking Issue

I've run across an issue with trying to link `libbrf.a` into one of
my programs, specifically [tls.c](../Client/tiny/tls.c).

The program uses `opendir()` and friends to read the contents of directories.
They, in turn, used `open()` and `read()` to read the directory contents.

The problem was that, when I compiled `tls.c` with `libbrf.a`, I was getting
the `lib.c` versions of `open()` and `read()` and not my shim functions.

Here is what is going on. Nowhere in the `tls.c` code is `open()` or `read()`
mentioned. So, when I run the compile command:

```
$ cc -o tls -O -i tls.c libbrf.a
```

the linker does not bring in `open()` or `read()` from `libbrf.a` as they
are not used by `tls.c`. After importing nothing from `libbrf.a` it moves
on to `libc.a`. When the linker brings in `opendir()` from `libc.a`, it now
knows that we need `open()` and `read()`. It imports `open()` and `read()`
from `libc.a`!

The solution is to pre-warn the linker that we need several functions in
`libbrf.a`. We use the `-u` flag to indicate that some functions are unknown
and will be needed:

```
$ cc -o tls -O -i tls.c -u _open -u _close -u _read libbrf.a
```

Many thanks to Steven Schultz and Johnny Billquist who patiently
explained this to me.

## Saving Space

Linking `libbrf.a` into your program adds about 10K of extra space.
This is because we need to use `gethostbyname()` to convert the
server name in `/etc/brf.conf` into a format we can use with
`connect()`.

There is a way to reduce this cost. In the client `Makefile`, add
`-DBINARY_CONFIGFILE` to the `CFLAGS` at the top, rebuild the
library and relink your program.

This causes the `open()` shim function to read the configuration from
a binary config file: `/etc/brf.bcnf`. The struct that we need to
send to `connect()` is already created and so we don't need
`gethostbyname()` and friends.

Now you need to make the `/etc/brf.bcnf` file. Assuming that you have
already made the text configuration file `/etc/brf.conf`, as `root`
run the `mkbrfcnf` program that is created along with the `libbrf.a`
library. This reads the text configuration file and builds the binary version.
