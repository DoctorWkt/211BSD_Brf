# Running `brfserver` As An Ordinary User

On the machine where I'm going to run `brfserver`, my login is:

```
$ id
uid=1000(wkt) gid=1000(wkt)
```

and all file/directories will get created with uid 1000 and gid 1000.
Over on my 2.11BSD system, I added the `wkt` username, uid, group name
and gid to `/etc/passwd` and `/etc/group` before I did the following.

On the machine where I'm going to run `brfserver`, I did:

```
$ mkdir /tmp/bar
$ ./brfserver -f /tmp/bar
```

I'll run the server in the foreground for now.

Over on my 2.11BSD system, I've logged in as `user`:

```
$ id
uid=100(user) gid=100(user) groups=100(user)
```

and there is an `/etc/brf.conf` file which points at the server:

```
$ cat /etc/brf.conf
/foo:eccles.local.net:1170
```

The "tiny" utilities are on my $PATH and are ready to use.
Let's just check if the normal utilities see `/foo`:

```
$ ls -l /foo
/foo not found
```

Of course not. Now let's try some "tiny" utilities:

```
$ tls -al /foo
drwxr-xr-x wkt      wkt         4096 .
drwxrwxrwx root     wheel      12288 ..
```

Yes! It's currently empty so let's fix that up.

```
$ tcp /etc/services /foo/file1
$ tls -l /foo
-rw-r--r-- wkt      wkt         1669 file1
```

Note that, although I am `user` on 2.11BSD, the files are owned
by `wkt` on the remote server. Let's see what is in this file:

```
$ tcat /foo/file1 | head
#       @(#)services    1.16.2 (2.11BSD) 1996/12/13
#
# Network services, Internet style
#
tcpmux          1/tcp          # TCP port multiplexer (RFC1078)
echo            7/tcp
echo            7/udp
discard         9/tcp           sink null
discard         9/udp           sink null
systat          11/tcp          users
```

We can `tcat` the file to standard output and use the normal `head` to
get the first few lines. What else? Let's change the file's permissions:

```
$ tchmod 444 /foo/file1
$ tls -l /foo/file1
-r--r--r-- wkt      wkt         1669 /foo/file1
```

Now let's try to overwrite the file:

```
$ tcp /etc/services /foo/file1
Write open failed: Permission denied
$ tchmod 644 /foo/file1
$ tcp /etc/services /foo/file1
```

Only when we have made the file writable can we do this. Let's rename it
and then remove it:

```
$ tmv /foo/file1 /foo/file2
$ tls -l /foo
-rw-r--r-- wkt      wkt         1669 file2

$ trm /foo/file1
Unlink failed: No such file or directory
$ trm /foo/file2

$ tls -l /foo
  (no output)
```

OK, let's copy a file to the remote server, copy it again and then link
it to have another filename:

```
$ tcp /etc/crontab /foo/newfile
$ tcp /foo/newfile /foo/another_file
$ tln /foo/another_file /foo/second_name

$ tls -l /foo
-rw-r--r-- wkt      wkt          268 another_file
-rw-r--r-- wkt      wkt          268 newfile
-rw-r--r-- wkt      wkt          268 second_name
  (I should make tls show link count)
```

Finally, some directories:

```
$ tmkdir /foo/fred
$ tls -l /foo
-rw-r--r-- wkt      wkt          268 another_file
drwxr-xr-x wkt      wkt         4096 fred
-rw-r--r-- wkt      wkt          268 newfile
-rw-r--r-- wkt      wkt          268 second_name
$ tcp /foo/second_name /foo/fred/lower_down

$ tls -l /foo/fred
-rw-r--r-- wkt      wkt          268 lower_down

$ trmdir /foo/fred
Rmdir failed: Destination address required
  (I need to map errnos, not done yet)

$ trm /foo/fred/lower_down
$ trmdir /foo/fred

$ tls -l /foo
-rw-r--r-- wkt      wkt          268 another_file
-rw-r--r-- wkt      wkt          268 newfile
-rw-r--r-- wkt      wkt          268 second_name
```
