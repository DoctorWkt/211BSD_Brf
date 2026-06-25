#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char *argv[]) {
  struct stat st;
  int err;

  if (argc != 2) {
    printf("Usage: %s file\n", argv[0]); exit(1);
  }

  err= stat(argv[1], &st);
  if (err == -1) {
    printf("Could not stat %s\n", argv[1]); exit(1);
  }

  printf("%s:\n", argv[1]);
  printf("  %d links, mode 0%o\n", st.st_nlink, st.st_mode);
  printf("  uid %d, gid %d\n", st.st_uid, st.st_gid);
  printf("  size %ld, %ld blocks, blksize %ld\n",
	st.st_size, st.st_blocks, st.st_blksize);
  printf("  ctime %s", ctime(&(st.st_ctime)));
  printf("  mtime %s", ctime(&(st.st_mtime)));
  printf("  atime %s", ctime(&(st.st_atime)));
  printf("  dev %d, rdev %d, ino %d\n",
	st.st_dev, st.st_rdev, st.st_ino);

  exit(0);
}
