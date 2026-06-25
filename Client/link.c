// Shim for link()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include "brf.h"

int link(char *path, char *path2) {
  int ret;

  // If the path starts with "/etc/" then
  // it is local. We need to allow gethostbyname() to work
  if (path != NULL && !strncmp(path, "/etc/", 5))
    return(syslink(path, path2));

  // Ditto path2
  if (path2 != NULL && !strncmp(path2, "/etc/", 5))
    return(syslink(path, path2));

  // No paths?
  if (path==NULL || path2==NULL) { errno= EFAULT; return(-1); }

  // Test the paths: are they remote or local?
  if ((brf_fname(path)== -1) || (brf_fname(path2)== -1)) {
    return(syslink(path,path2));		// local
  }

  // It is remote, send the BRF command
  if (brf_send(BRF_OPLINK, path, strlen(path)+1,
			path2, strlen(path2)+1)<0)
    return(-1);

  // Now get the response
  ret= brf_recv(NULL, NULL);
#ifdef DEBUG
  syslog(LOG_DEBUG, "link(%s, %s) returned %d", path, path2, ret);
#endif
  return(ret);
}
