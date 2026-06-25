// Shim for dup2()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include "brf.h"

int dup2(int fd, int fd2) {
  int ret;

  // Deal with local file descriptors
  if (fd < BRF_FDOFFSET)
    return(sysdup2(fd, fd2));

  // Send the request
  if (brf_send(BRF_OPDUP2, &fd, sizeof(fd) + sizeof(fd2), NULL, 0)<0)
    return(-1);

  // Get the response
  ret=brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "dup2(%d, %d) returned %d", fd, fd2, ret);
#endif
  return(ret);
}
