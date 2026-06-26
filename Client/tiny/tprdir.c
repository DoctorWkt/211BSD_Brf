// Print the contents of a directory

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/param.h>
#include <sys/dir.h>

int main(int argc, char *argv[]) {
  struct stat st;
  int err;
  DIR *dirp;
  struct direct *dp;

  if (argc != 2) {
    printf("Usage: %s dir\n", argv[0]); exit(1);
  }

  err= stat(argv[1], &st);
  if (err == -1) {
    printf("Could not stat %s\n", argv[1]); exit(1);
  }

  if (!S_ISDIR(st.st_mode)) {
    printf("%s is not a directory\n", argv[1]); exit(1);
  }

  if ((dirp = opendir(argv[1])) == NULL) {
    printf("Could not opendir %s\n", argv[1]); exit(1);
  }

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    printf("%s\n", dp->d_name);
  }

  closedir(dirp);

  exit(0);
}
