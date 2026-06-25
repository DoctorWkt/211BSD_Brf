// Print the contents of a directory
//
// Not sure why I have to put the
// source for opendir() and friends here.
// I'm guessing a link ordering issue.

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/param.h>
#include <sys/dir.h>

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	char	*malloc();

	if ((fd = open(name, 0)) == -1)
		return NULL;
	if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
		close (fd);
		return NULL;
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return dirp;
}

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct direct *dp;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_size <= 0)
				return NULL;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > DIRBLKSIZ + 1 - dirp->dd_loc)
			return NULL;
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}

/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
	free(dirp);
}

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
