# Linking `libbrf.a` Into Your Program

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
