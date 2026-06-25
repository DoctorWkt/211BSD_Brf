// Tiny version of rm(1)
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

  if (argc < 2) {
    printf("Usage %s file [file ...]\n", argv[0]); exit(1);
  }

  for (i=1; i < argc; i++) {
    result= unlink(argv[i]);
    if (result == -1) perror("Unlink failed");
  }

  exit(0);
}
