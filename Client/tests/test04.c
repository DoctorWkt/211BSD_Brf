#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>

#define FILENAME "/foo/abcd"

char *mesg= "Hello from a 2.11BSD system!\n";
char *buf=  "                                              ";

int main(int argc, char *argv[]) {
  int fd;
  int result;
  struct stat st;

  printf("Opening %s O_WRONLY|O_CREAT\n", FILENAME);
  fd= open(FILENAME, O_WRONLY|O_CREAT, 0644);
  if (fd == -1) { perror("Open failed"); exit(1); }

  printf("Writing %d bytes to the file\n", strlen(mesg));
  result= write(fd, mesg, strlen(mesg));
  if (result == -1) { perror("Write failed"); exit(1); }
  printf("Wrote %d bytes\n", result);

  printf("Closing the fd\n");
  result= close(fd);
  if (result == -1) { perror("Close failed"); exit(1); }

  // Now let's try to read it back!
  printf("Opening %s O_RDONLY\n", FILENAME);
  fd= open(FILENAME, O_RDONLY, 0);
  if (fd == -1) { perror("Open failed"); exit(1); }

  printf("Reading from the file\n");
  result= read(fd, buf, strlen(buf));
  if (result < 1) { perror("Read failed"); exit(1); }
  printf("We read in %d bytes\n", result);
  write(1, buf, result);
  
  result= fstat(fd, &st);
  if (result == -1) { perror("Fstat failed"); exit(1); }
  printf("fstat: size %ld\n", st.st_size);

  printf("Closing the fd\n");
  result= close(fd);
  if (result == -1) { perror("Close failed"); exit(1); }

  result= stat(FILENAME, &st);
  if (result == -1) { perror("Stat failed"); exit(1); }
  printf("stat: size %ld\n", st.st_size);

  result= lstat(FILENAME, &st);
  if (result == -1) { perror("Lstat failed"); exit(1); }
  printf("lstat: size %ld\n", st.st_size);

  exit(0);
}
