// Tiny version of ln(1)
// (c) 2026 Warren Toomey, GPL3

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  int i;
  int result;

  if (argc != 3) {
    printf("Usage %s file newfile\n", argv[0]); exit(1);
  }

  result= link(argv[1], argv[2]);
  if (result == -1) perror("Link failed");

  exit(0);
}
