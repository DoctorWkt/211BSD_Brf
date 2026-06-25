// Shim for fchmod()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int fchmod(int fd, int mode) {
  int ret;

  // Write local file descriptors
  if (fd < BRF_FDOFFSET) {
    return(sysfchmod(fd, mode));
  }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPFCHMOD, &fd,
	sizeof(fd) + sizeof(mode), NULL, 0)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "fchmod(%d, 0%o) returned %d",
	fd, mode, ret);
#endif
  return(ret);
}
