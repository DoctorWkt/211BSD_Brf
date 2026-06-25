// Shim for close()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include "brf.h"

int close(int fd) {
  int ret;

  // Close local file descriptors
  if (fd < BRF_FDOFFSET)
    return(sysclose(fd));

  // Send the request
  if (brf_send(BRF_OPCLOSE, &fd, sizeof(fd), NULL, 0)<0)
    return(-1);

  // Get the response
  ret=brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "close(%d) returned %d", fd, ret);
#endif
  return(ret);
}
