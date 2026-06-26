// Tiny version of cat(1)
// (c) 2026 Warren Toomey, GPL3

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define BUFSIZE 16384

int main(int argc, char *argv[]) {
  int i;
  int fd;
  int result;
  int buf[BUFSIZE];

  if (argc < 2) {
    printf("Usage %s file [file ...]\n", argv[0]); exit(1);
  }

  for (i=1; i < argc; i++) {
    fd= open(argv[i], O_RDONLY, 0);
    if (fd == -1) { perror("Read open failed"); continue; }

    while (1) {
      result= read(fd, buf, BUFSIZE);
      if (result == -1) { perror("Read failed"); break; }
      if (result ==0) break;
      result= write(1, buf, result);
      if (result == -1) { perror("Write failed"); break; }
    }
    close(fd);
  }

  exit(0);
}
