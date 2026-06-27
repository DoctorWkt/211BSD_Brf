// Shim for rename()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int rename(char *path, char *path2) {
  int ret;

  // Ditto path2
  if (path2 != NULL && !strncmp(path2, "/etc/", 5))
    return(sysrename(path, path2));

  // No paths?
  if (path==NULL || path2==NULL) { errno= EFAULT; return(-1); }

  // Test the paths: are they remote or local?
  if ((brf_fname(path)== -1) || (brf_fname(path2)== -1)) {
    return(sysrename(path,path2));		// local
  }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPRENAME, path, strlen(path)+1,
			path2, strlen(path2)+1)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "rename(%s, %s) returned %d", path, path2, ret);
#endif
  return(ret);
}
