// Shim for chmod()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int chmod(char *path, int mode) {
  int ret;

  // If the path starts with "/etc/" then
  // it is local. We need to allow gethostbyname() to work
  if (path != NULL && !strncmp(path, "/etc/", 5))
    return(syschmod(path, mode));

  // No path?
  if (path==NULL) { errno= EFAULT; return(-1); }

  // Test the path: is it remote or local?
  if (brf_fname(path)== -1) {
    return(syschmod(path, mode));		// local
  }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPCHMOD, &mode, sizeof(mode),
	path, strlen(path)+1)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "chmod(%s, 0%o) returned %d",
	path, mode, ret);
#endif
  return(ret);
}
