// Tiny version of chmod(1)
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
  int c;
  char *ms;
  int mode=0;
  int result;

  if (argc < 3) {
    printf("Usage %s octal_mode [file ...]\n", argv[0]); exit(1);
  }
  ms= argv[1];

  // Get the octal mode
  while ((c = *ms++) >= '0' && c <= '7')
    mode = (mode << 3) + (c - '0');

  for (i=2; i < argc; i++) {
    result= chmod(argv[i], mode);
    if (result == -1) perror("Chmod failed");
  }

  exit(0);
}
