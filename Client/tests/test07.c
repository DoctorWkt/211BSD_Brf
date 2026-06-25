#include <sys/types.h>
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

  printf("Opening %s O_WRONLY|O_CREAT\n", FILENAME);
  fd= open(FILENAME, O_WRONLY|O_CREAT, 0644);
  if (fd == -1) { perror("Open failed"); exit(1); }

  result= write(fd, mesg, strlen(mesg));
  if (result == -1) { perror("Write failed"); exit(1); }
  result= close(fd);
  if (result == -1) { perror("Close failed"); exit(1); }

  // Change permissions
  printf("Set permission to 0400\n");
  result= chmod(FILENAME, 0400);
  if (result == -1) { perror("Chmod failed"); exit(1); }
  printf("Set permission to 0664\n");
  result= chmod(FILENAME, 0664);
  if (result == -1) { perror("Chmod failed"); exit(1); }
  exit(0);
}
