## Limitations of Brf

Because Brf is a set of "shim" functions, it only works when a utility
is linked with the Brf library. At present this is only the "tiny"
utilities. Other programs not linked with the Brf library, e.g. the shells,
`tar` etc. cannot "see" the remote filesystem. Thus, you cannot use
the "glob" shell operators (e.g. '*', '?', '[ ... ]'):

```
$ tls -l /foo
-rw-r--r-- wkt      wkt          110 Makefile
-r-------- wkt      wkt           29 abcd
-rw-r--r-- wkt      wkt       245760 brf.tar
$ tls -l /foo/*.tar
tls: No match.
```

(as the shell cannot expand `/foo/*.tar`).

There is an issue with the order of functions at the link stage:
I've had to include the source code to `opendir()` and friends
in the tiny `ls` utility. When I left the `opendir()` source out and did:

```
cc -o tls tls.c ../librf.a
```

then `opendir()` (which is in `libc` and which gets linked in _after_
the Brf library) sees the real `open()` and not the "shim" version of
`open()`. I would really like find a solution to this so that we can
link any existing utility with the Brf library and other libraries
(e.g. `libc` and hence the "stdio" library) will see the shim functions.

The performance of Brf, both latency and data throughput, is poor.
This is most likely because I am using TCP. Each client request requires
a TCP ACK and each server response requires a TCP ACK, which adds latency
to successive file operations. And, as TCP has slow-start, it can take a
while for the speed of a data transfer to ramp up. One solution would be
to switch over to UDP but then we have to deal with packet retransmissions
and idempotency.

There is no security. Clients do not have to authenticate with the server,
so any machine that can connect to the server can access files on the server.
Also, pathnames could contain multiple "../", which means that a client
request can access files all the way up to the root of the server system
and beyond. And, if you run the server as `root`, then __every__ file on
the server filesystem becomes available.

Remote pathnames, at present, _must_ be absolute, i.e. start with a '/'. It
is technically possible to deal with relative pathnames, but I wanted to get
the main functionality working before I even tried to support this.

For some reason, data reads bigger than ~1,024 bytes from the server seem
to fail sporadically. In `read.c` I've limited reads to 1,024 bytes.
