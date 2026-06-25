// Shim for dup()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include "brf.h"

int dup(int fd) {
  int ret;

  // Deal with local file descriptors
  if (fd < BRF_FDOFFSET)
    return(sysdup(fd));

  // Send the request
  if (brf_send(BRF_OPDUP, &fd, sizeof(fd), NULL, 0)<0)
    return(-1);

  // Get the response
  ret=brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "dup(%d) returned %d", fd, ret);
#endif
  return(ret);
}
