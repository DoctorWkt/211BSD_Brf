// Shim for lseek()
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include "brf.h"

off_t lseek(int fd, off_t offset, int whence) {
  int ret, size;
  struct brf_seek bseek;

  // Local file descriptor?
  if (fd < BRF_FDOFFSET)
    return(syslseek(fd, offset, whence));

  // Build the request
  bseek.fd= fd;
  bseek.offset= offset;
  bseek.whence= whence;

  // Send the request
  if (brf_send(BRF_OPLSEEK, &bseek, sizeof(bseek), NULL, 0)<0)
    return(-1);

  // Get the response; get the new offset
  ret=brf_recv(&bseek, &size);

#ifdef DEBUG
  syslog(LOG_DEBUG, "lseek(%d, %ld", bseek.fd, bseek.offset);
#endif
  if (ret == -1) bseek.offset= -1;
#ifdef DEBUG
  syslog(LOG_DEBUG, ", %d) returned %ld", bseek.whence, bseek.offset);
#endif
  return(bseek.offset);
}
