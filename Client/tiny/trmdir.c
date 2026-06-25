// Tiny version of rmdir(1)
// (c) 2026 Warren Toomey, GPL3

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  int result;

  if (argc != 2) {
    printf("Usage %s dir\n", argv[0]); exit(1);
  }

  result= rmdir(argv[1]);
  if (result == -1) perror("Rmdir failed");

  exit(0);
}
