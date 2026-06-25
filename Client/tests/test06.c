#include <sys/types.h>
#include <sys/file.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define FILENAME "/foo/fred"

char *mesg= "Hello from a 2.11BSD system!\n";
char *buf=  "                                              ";

int main(int argc, char *argv[]) {
  int fd;
  int result;

  printf("Opening %s O_WRONLY|O_CREAT mode 0544\n", FILENAME);
  fd= open(FILENAME, O_WRONLY|O_CREAT, 0544);
  if (fd == -1) { perror("Open failed"); exit(1); }

  result= write(fd, mesg, strlen(mesg));
  if (result == -1) { perror("Write failed"); exit(1); }
  result= close(fd);
  if (result == -1) { perror("Close failed"); exit(1); }

  // Test access
  result= access(FILENAME, F_OK);
  printf("F_OK access gives %d\n", result);
  result= access(FILENAME, R_OK);
  printf("R_OK access gives %d\n", result);
  result= access(FILENAME, W_OK);
  printf("W_OK access gives %d\n", result);
  result= access(FILENAME, X_OK);
  printf("X_OK access gives %d\n", result);

  exit(0);
}
