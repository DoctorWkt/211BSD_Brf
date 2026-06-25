# The Brf Protocol

Brf clients and the Brf server exchange requests and responses over
a TCP connection; each client gets its own TCP connection. Data is
sent in binary 2.11BSD format: little-endian 16-bit integers, PDP-11
format 32-bit integers and NUL-terminated pathnames.

## Client Requests

Each client request starts with this header:

```
struct brf_req {        // Client request
  uint8_t  flag;        // Must be 0xFF
  uint8_t  cmd;         // Command
  uint16_t len;         // Length of any following data
};
```

The `flag` is used by the server to ensure that it is in sync with the
requests sent by the client: if the `flag`'s value is not 0xFF then
this is not a valid client request.

The request holds the client's command and the amount of data following.
The header is then followed by any fixed-length data fields (file
descriptors, file modes etc.); this is then followed by any variable-length
data, usually NUL-terminated pathnames.

Let's look at an example. Say the client wants to open a file:

```
   open("/foo/file.txt", O_WRONLY|O_CREAT, 0644);
```

The `O_WRONLY|O_CREAT` value becomes the octal number 0201. The Brf
request would therefore be:

 + Flag 0xFF
 + Cmd 2, i.e. BRF_OPEN
 + Len 18, the amount of data bytes following.
 + The 16-bit value 0201, i.e. the open flags.
 + The 16-bit value 0644, i.e. the mode.
 + The NUL-terminated pathname "/foo/file.txt".

The first command a client sends to the server is the BRF_OPINIT command:

```
struct brf_init {       // BRF_OPINIT data
  uint16_t  uid;        // uid of process
  uint16_t  gid;        // gid of process
                        // then the base directory on the client
};
```

So, if the file `/etc/brf.conf` has the base directory "/foo" and the
client is running with user-id 100 and group-id 100, then these values
are sent with the BRF_OPINIT request.

If the server is running as `root`, then the user-id and group-id of the
server are changed to those sent in the BRF_OPINIT request.

I'm not going to document all the available Brf client requests.
The best way to see them is to read the source code of all the
"shim" functions that get compiled and put into the `libbrf.a` library.

## Server Responses

Once a server has performed a client's request, it returns a response
to the client. This starts with the header below and is followed by
any fixed-length data fields and then any variable-length data fields.

```
struct brf_resp {       // Server response
  uint8_t  flag;        // Must be 0xFF
  uint8_t  cmd;         // Command, echoed back
  int16_t  result;      // Result of operation
  uint16_t err;         // Errno value
  uint16_t len;         // Length of any following data
};
```

We again have an 0xFF `flag` for synchronisation reasons.
The client's command is sent back although, at present, it is not used
in the client code.

If the `err` value is not zero, then the client will set `errno` to this
value. Finally, the `result` value is returned to the caller of the shim
function on the client.

Again, I'm not going to document all the available Brf server responses.
The best way to see them is to read the source code of all the
"shim" functions that get compiled and put into the `libbrf.a` library.
