## Limitations of Brf

Because Brf is a set of "shim" functions, it only works when a utility
is linked with the Brf library. At present this is only the "tiny"
utilities. Other programs not linked with the Brf library, e.g. the shells,
`tar` etc. cannot "see" the remote filesystem. Thus, you cannot use
the "glob" shell operators (e.g. '*', '?', '[ ... ]'):

```
$ tls -l /foo
-rw-r--r-- 1 wkt      wkt          110 Makefile
-r-------- 1 wkt      wkt           29 abcd
-rw-r--r-- 1 wkt      wkt       245760 brf.tar
$ tls -l /foo/*.tar
tls: No match.
```

(as the shell cannot expand `/foo/*.tar`).

If you are linking your program against `libbrf.a`, you will need to
keep [this information](Linking.md) in mind.

There is no security. Clients do not have to authenticate with the server,
so any machine that can connect to the server can access files on the server.
Also, pathnames could contain multiple "../", which means that a client
request can access files all the way up to the root of the server system
and beyond. And, if you run the server as `root`, then __every__ file on
the server filesystem becomes available.

Remote pathnames, at present, _must_ be absolute, i.e. start with a '/'. It
is technically possible to deal with relative pathnames, but I wanted to get
the main functionality working before I even tried to support this.
