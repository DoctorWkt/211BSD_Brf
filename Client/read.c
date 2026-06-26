// Shim for read()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int read(int fd, char *buf, unsigned short nbytes) {
  struct brf_rw brw;
  int ret, cnt;

  // Write local file descriptors
  if (fd < BRF_FDOFFSET) {
    return(sysread(fd, buf, nbytes));
  }

  // No buffer
  if (buf==NULL) { errno= EFAULT; return(-1); }

  // XXX: For some reason, large read requests fail
  if (nbytes > 1024) nbytes= 1024;

  // Send the request
  brw.fd= fd;
  brw.nbytes= nbytes;
  if (brf_send(BRF_OPREAD, &brw, sizeof(brw), NULL, 0)<0)
    return(-1);

  // Get the response
  ret=brf_recv(buf, &cnt);
#ifdef DEBUG
  syslog(LOG_DEBUG, "read(%d, ..., %d) returned %d", fd, nbytes, ret);
#endif
  return(ret);
}
