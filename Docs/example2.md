# Running `brfserver` As Root

Before you read this, please read through [example1.md](example1.md)
which shows `brfserver` running as an ordinary user.

Now I'm going to become `root` and run `brfserver` on my server:

```
$ sudo bash
 ...
# ./brfserver -f /tmp/bar
```

Over on my 2.11BSD system, I will login as `root` and ensure that the
"tiny" utilities are on my $PATH:

```
# id
uid=0(root) gid=1(daemon) groups=1(daemon), 0(wheel)
```

Let's see what is in the remote directory:

```
# tls -al /foo
drwxr-xr-x 2 wkt      wkt         4096 .
drwxrwxrwx 2 root     wheel      12288 ..
-rw-r--r-- 2 wkt      wkt          268 another_file
-rw-r--r-- 1 wkt      wkt          268 newfile
-rw-r--r-- 2 wkt      wkt          268 second_name
```

Now we can change ownership of some files:

```
# tchown root.wheel /foo/second_name
# tls -al /foo
drwxr-xr-x 2 wkt      wkt         4096 .
drwxrwxrwx 2 root     wheel      12288 ..
-rw-r--r-- 2 root     wheel        268 another_file
-rw-r--r-- 1 wkt      wkt          268 newfile
-rw-r--r-- 2 root     wheel        268 second_name
```

Note that both `second_name` and `another_file` had their
ownership changed. That is because they are hard links to the
same underlying file, and we changed the file's ownership.
