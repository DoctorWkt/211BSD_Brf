// Tiny version of chown(1)
// (c) 2026 Warren Toomey, GPL3
// Some code borrowed from 2.11BSD chown(1)

#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

int isnumber(char *s) {
  int c;

  while (c = *s++)
    if (!isdigit(c)) return (0);
  return (1);
}

int main(int argc, char *argv[]) {
  int i;
  int result;
  int uid, gid;
  char *group;
  struct group *grp;
  struct  passwd *pwd;

  if (argc < 3) {
    printf("Usage %s uid.gid file [file ...]\n", argv[0]); exit(1);
  }

  // Get the gid
  gid = -1;
  group = index(argv[1], '.');
  if (group != NULL) {
    *group++ = '\0';
    if (!isnumber(group)) {
      if ((grp = getgrnam(group)) == NULL) {
        fprintf(stderr, "Unknown group %s\n", group); exit(1);
      }
      gid = grp -> gr_gid;
      endgrent();
    } else if (*group != '\0')
        gid = atoi(group);
  }

  // Get the uid
  if (!isnumber(argv[1])) {
    if ((pwd = getpwnam(argv[1])) == NULL) {
      fprintf(stderr, "unknown user id: %s",argv[1]); exit(1);
    }
    uid = pwd->pw_uid;
  } else
    uid = atoi(argv[1]);

  for (i=2; i < argc; i++) {
    result= chown(argv[i], uid, gid);
    if (result == -1) perror("Chown failed");
  }

  exit(0);
}
