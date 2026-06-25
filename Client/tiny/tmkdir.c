// Tiny version of mkdir(1)
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
    printf("Usage %s newdir\n", argv[0]); exit(1);
  }

  result= mkdir(argv[1], 0755);
  if (result == -1) perror("Mkdir failed");

  exit(0);
}
