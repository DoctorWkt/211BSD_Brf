#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define FILENAME "/foo/seektest"

char *mesg= "Hello from a 2.11BSD system!\n"
	"Shakespeare's sonnets are considered a\n"
	"continuation of the sonnet tradition that\n"
	"swept through the Renaissance from Petrarch\n"
	"in 14th-century Italy and was finally\n"
	"introduced in 16th-century England by Thomas\n"
	"Wyatt and was given its rhyming metre and\n"
	"division into quatrains by Henry Howard.\n";

char *buf=  "                                              ";

int main(int argc, char *argv[]) {
  int fd;
  int result;
  off_t offset;

  printf("Opening %s O_RDWR|O_CREAT\n", FILENAME);
  fd= open(FILENAME, O_RDWR|O_CREAT, 0644);
  if (fd == -1) { perror("Open failed"); exit(1); }

  printf("Writing %d bytes to the file\n", strlen(mesg));
  result= write(fd, mesg, strlen(mesg));
  if (result == -1) { perror("Write failed"); exit(1); }
  printf("Wrote %d bytes\n", result);

  printf("Seeking to the start of the file\n");
  offset= lseek(fd, 0L, SEEK_SET);
  if (offset == -1) { perror("Lseek failed"); exit(1); }

  result= read(fd, buf, 10);
  if (result < 1) { perror("Read failed"); exit(1); }
  write(1, buf, result); printf("\n");
  
  printf("Seeking to the middle of the file\n");
  offset= lseek(fd, 100L, SEEK_CUR);
  if (offset == -1) { perror("Lseek failed"); exit(1); }

  result= read(fd, buf, 10);
  if (result < 1) { perror("Read failed"); exit(1); }
  write(1, buf, result); printf("\n");
  
  printf("Seeking to the end of the file\n");
  offset= lseek(fd, -20L, SEEK_END);
  if (offset == -1) { perror("Lseek failed"); exit(1); }

  result= read(fd, buf, 10);
  if (result < 1) { perror("Read failed"); exit(1); }
  write(1, buf, result); printf("\n");
  
  printf("Closing the fd\n");
  result= close(fd);
  if (result == -1) { perror("Close failed"); exit(1); }

  exit(0);
}
