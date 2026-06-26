// Tiny version of to(1): write stdin to a file.
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
  int fd;
  int result;
  int buf[BUFSIZE];

  if (argc != 2) {
    printf("Usage %s file\n", argv[0]); exit(1);
  }

  fd= open(argv[1], O_WRONLY|O_CREAT, 0644);
  if (fd == -1) { perror("Write open failed"); exit(1); }

  while (1) {
    result= read(0, buf, BUFSIZE);
    if (result == -1) { perror("Read failed"); exit(1); }
    if (result ==0) break;
    result= write(fd, buf, result);
    if (result == -1) { perror("Write failed"); exit(1); }
  }

  close(fd);
  exit(0);
}
