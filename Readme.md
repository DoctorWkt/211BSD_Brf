# 211BSD_Brf: A Basic Remote Filesystem for 2.11BSD

_Note: This is_ __NOT__ _NFS!_

This is a work-in-progress, proof of concept, implementation
of a remote filesystem for 2.11BSD.

Instead of trying to add a remote filesystem in the kernel,
I have created a set of "shim" functions that sit in front
of the 2.11BSD file-oriented system calls. If a shim function
sees that a file (or file-descriptor) is remote, it sends a
request to the remote file server which does the work on that system.

There are significant limitations, see the [Limitations.md](Docs/Limitations.md)
file for details. For now, you get a small set of "tiny" utilities that
can see the remote filesystem; all the existing utilities cannot.
Also, the latency and transfer speeds need to be improved.

You also get the Brf library, `libbrf.a`, which holds all the "shim"
functions if you want to link them in to your own utility.

## Server Installation

On the machine you want to be the Brf server, change into
the [Server](Server) directory and do a `make` to create
the `brfserver` executable.

```
$ make
cc -o brfserver -O netcode.c bsdcode.c
```

Make a directory somewhere on the server, e.g:

```
$ mkdir /tmp/bar
```

This directory and its contents will be visible on your 2.11BSD machine.
Run the `brfserver` executable and name this directory:

```
$ ./brfserver /tmp/bar
$
```

The server becomes a daemon and waits for connections on TCP port 1170.
Alternatively, you can run `$ ./brfserver -f /tmp/bar` and the server will
stay running in the foreground until you terminate it with ctrl-C.

## Client Installation

Over on your 2.11BSD, import the executables in the 
[Tinybins](Tinybins) directory and put them somewhere on your `$PATH`.
All the executables start with the `t` letter: `tcp`, `tcat`, `tstat`,
`tprdir`, `tls`, `trm`, `tln`, `tmv`, `tmkdir`, `trmdir`, `tchmod` etc.
Use whatever you need to do this: `ftp`, a "tap" tarball etc.

Once you have them, future file transfers will be easier :-)

## Client Configuration

Ensure that the name and IP address of the machine running the `brfserver`
are in your `/etc/hosts` file or are known via DNS lookup. On my 2.11BSD
machine my `/etc/hosts` file has:

```
10.10.1.45      eccles.local.net eccles
```

Create an `/etc/brf.conf` which has three colon-separated fields:
the location where the remote directory will be visible on 2.11BSD,
the name and the TCP port of the `brfserver` machine. On my 2.11BSD
machine my `/etc/brf.conf` file has:

```
/foo:eccles.local.net:1170
```

The `/foo` directory, which does not even exist on 2.11BSD, will
become visible to the "tiny" utilities with the Brf functions compiled in.

## Examples of Use

The 2.11BSD "tiny" utilities send the current user-id/group-id when
they first connect to the `brfserver`.

If you run the `brfserver` as a non-root user, then the user-id/group-id
gets ignored; every file and directory access/made by the 2.11BSD clients
will be owned by the user running `brfserver`. Here is an
[example](Docs/example1.md) of the 2.11BSD "tiny" utilities
being used in this mode.

If you run the `brfserver` as `root`, then the user-id/group-id will be
used to access/create files on the `brfserver` remote directory.
Here is an [example](Docs/example2.md) of the 2.11BSD "tiny" utilities
being used in this mode.

As there is no authentication in the Brf protocol, anybody who can send
requests to your `brfserver` will be able to see files on your server.
I strongly recommend that you __DO NOT__ run `brfserver` as `root`!
See the [Limitations.md](Docs/Limitations.md) file for details.

## Documentation

  + [Limitations of Brf](Docs/Limitations.md)
  + [Details of the Brf protocol](Docs/Protocol.md)
  + [A to-do list](Docs/TODO.md)

The tiny utilities have no man pages. Please run them with no arguments
and/or read their source code for details.

## Source Code

  + The [Client/](Client) directory has the source to the Brf library
    and the tiny utilities.
  + The [Server/](Server) directory has the source for the Brf server.

## Debugging

If you compile the Brf library with `-DDEBUG`, then many of the "shim"
functions will log their activity using the `LOG_USER` facility. On
2.11BSD, you should be able to see this in the `/usr/adm/debuglog` file.

Similarly, if you compile the `brfserver` with `-DDEBUG`, then it will
also log its activity using the `LOG_USER` facility. Consult your
server system's log configuration to determine where this information
gets logged.

On the client side there are a few tests in the 
[Client/tests/](Client/tests) directory.
