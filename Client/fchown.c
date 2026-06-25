// Shim for fchown()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int fchown(int fd, int uid, int gid) {
  int ret;

  // Fchown local file descriptors
  if (fd < BRF_FDOFFSET)
    return(sysfchown(fd, uid, gid));

  // It is remote, send the BRF command
  if (brf_send(BRF_OPFCHOWN, &fd,
	sizeof(fd) + sizeof(uid) + sizeof(gid), NULL, 0)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "fchown(%d, %d, %d) returned %d",
	fd, uid, gid, ret);
#endif
  return(ret);
}
