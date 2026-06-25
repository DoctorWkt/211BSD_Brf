// Tiny version of cp(1)
// (c) 2026 Warren Toomey, GPL3

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  int fd, fd2;
  int result;
  int buf[16384];

  if (argc != 3) {
    printf("Usage %s file newfile\n", argv[0]); exit(1);
  }

  fd= open(argv[1], O_RDONLY, 0);
  if (fd == -1) { perror("Read open failed"); exit(1); }

  fd2= open(argv[2], O_WRONLY|O_CREAT, 0644);
  if (fd2 == -1) { perror("Write open failed"); exit(1); }

  while (1) {
    result= read(fd, buf, 16384);
    if (result == -1) { perror("Read failed"); exit(1); }
    if (result ==0) break;
    result= write(fd2, buf, result);
    if (result == -1) { perror("Write failed"); exit(1); }
  }

  close(fd); close(fd2); exit(0);
  exit(0);
}
