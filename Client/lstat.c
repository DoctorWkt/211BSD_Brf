// Shim for lstat()

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include "brf.h"

int lstat(char *path, struct stat *buf) {
  int ret;
  int num;

  // If the path starts with "/etc/" then
  // it is local. We need to allow gethostbyname() to work
  if (path != NULL && !strncmp(path, "/etc/", 5))
    return(syslstat(path, buf));

  // No path or buf?
  if (path==NULL) { errno= EFAULT; return(-1); }
  if (buf==NULL) { errno= EFAULT; return(-1); }

  // Test the path: is it remote or local?
  if (brf_fname(path)== -1) {
    return(syslstat(path, buf));         // local
  }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPLSTAT, path, strlen(path)+1, NULL, 0)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(buf, &num);
#ifdef DEBUG
  syslog(LOG_DEBUG, "lstat(%s) returned %d", path, ret);
#endif
  return(ret);
}
