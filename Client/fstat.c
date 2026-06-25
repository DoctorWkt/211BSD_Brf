// Shim for fstat()

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include "brf.h"

int fstat(int fd, struct stat *buf) {
  int ret;
  int num;

  // Stat local file descriptors
  if (fd < BRF_FDOFFSET) {
    return(sysfstat(fd, buf));
  }

  // No buf?
  if (buf==NULL) { errno= EFAULT; return(-1); }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPFSTAT, &fd, sizeof(fd), NULL, 0)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(buf, &num);
#ifdef DEBUG
  syslog(LOG_DEBUG, "fstat(%d) returned %d", fd, ret);
#endif
  return(ret);
}
