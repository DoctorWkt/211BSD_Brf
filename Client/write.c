// Shim for write()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int write(int fd, char *buf, unsigned short nbytes) {
  struct brf_rw brw;
  int ret;

  // Write local file descriptors
  if (fd < BRF_FDOFFSET)
    return(syswrite(fd, buf, nbytes));

  // No buffer
  if (buf==NULL) { errno= EFAULT; return(-1); }

  // Send the request
  brw.fd= fd;
  brw.nbytes= nbytes;
  if (brf_send(BRF_OPWRITE, &brw, sizeof(brw), buf, nbytes)<0)
    return(-1);

  // Get the response
  ret=brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "write(%d, ..., %d) returned %d", fd, nbytes, ret);
#endif
  return(ret);
}
