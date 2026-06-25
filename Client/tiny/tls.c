#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/dir.h>
#include <stdlib.h>
#include <string.h>
#include <sgtty.h>
#include <strings.h>
#include <sys/time.h>
#include <utmp.h>
#include <pwd.h>
#include <grp.h>

struct  utmp utmp;
#define NMAX    (sizeof (utmp.ut_name))
#define SCPYN(a, b)     strncpy(a, b, NMAX)
#define NCACHE  64       /* power of 2 */
#define CAMASK  NCACHE - 1

/* ls: List directory contents.		Author: Warren Toomey */

/* External variables. */
extern int optind;
extern char *optarg;

int longoutput=0;		// Do a long (-l) output
int showdots=0;			// Show dot files
int showinums=0;		// Show i-numbers not link counts
int dircontents=1;		// Show the contents of a directory


// Not sure why I have to put the
// source for opendir() and friends here.
// I'm guessing a link ordering issue.

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;

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

char *
getname(uid)
        uid_t uid;
{
        static struct ncache {
                uid_t   uid;
                char    name[NMAX+1];
        } c_uid[NCACHE];
        register struct passwd *pw;
        register struct ncache *cp;

        setpassent(1);
        cp = c_uid + (uid & CAMASK);
        if (cp->uid == uid && *cp->name)
                return(cp->name);
        if (!(pw = getpwuid(uid)))
                return((char *)0);
        cp->uid = uid;
        SCPYN(cp->name, pw->pw_name);
        return(cp->name);
}

char *
getgroup(gid)
        gid_t gid;
{
        static struct ncache {
                gid_t   gid;
                char    name[NMAX+1];
        } c_gid[NCACHE];
        register struct group *gr;
        register struct ncache *cp;

        cp = c_gid + (gid & CAMASK);
        if (cp->gid == gid && *cp->name)
                return(cp->name);
        if (!(gr = getgrgid(gid)))
                return((char *)0);
        cp->gid = gid;
        SCPYN(cp->name, gr->gr_name);
        return(cp->name);
}

// Human readable permissions strings
char *permstr[8] = {
  "---", "--x", "-w-", "-wx",
  "r--", "r-x", "rw-", "rwx"
};

void listone(char *entry, struct stat *sbptr)
{
  char ftype;
  char *cp;

  if (longoutput) {
    // Print the type of entry as the first character
    switch (sbptr->st_mode& S_IFMT) {
      case S_IFDIR: ftype='d'; break;
      case S_IFREG: ftype='-'; break;
      default: ftype='-';
    }

    printf("%c", ftype);
    printf("%s",  permstr[(sbptr->st_mode >> 6) & 7]);
    printf("%s",  permstr[(sbptr->st_mode >> 3) & 7]);
    printf("%s ", permstr[(sbptr->st_mode)      & 7]);

    cp = getname(sbptr->st_uid);
    if (cp) printf("%-9.9s", cp);
    else    printf("%-9u", sbptr->st_uid);

    cp = getgroup(sbptr->st_gid);
    if (cp) printf("%-9.9s", cp);
    else    printf("%-9u", sbptr->st_gid);

    printf(" %6ld %s\n", sbptr->st_size, entry);
  } else {
    // Just print out the name
    puts(entry);
  }
}

// We keep a array structure of directs and their stat buffers
typedef struct {
  struct direct *dent;
  struct stat sb;
} Namestat;

#define NLISTSIZE 200
Namestat namelist[NLISTSIZE];

// Compare two Namestat structs using the entry names
int namecmp(const void *a, const void *b)
{
  Namestat *aa= (Namestat *)a;
  Namestat *bb= (Namestat *)b;

  return(strcmp(aa->dent->d_name, bb->dent->d_name));
}

// List the entry (if a file), or its contents (if a directory)
void listmany(char *entry)
{
  char buf[MAXPATHLEN + 1], *p;
  DIR *D;
  struct direct *dent;
  struct stat sb;
  int count=0;
  int i;

  // Ensure the entry exists
  if (stat(entry, &sb)==-1) {
    printf("%s: non-existent\n", entry);
    return;
  }

  // It's a file, just print it out
  if (S_ISREG(sb.st_mode)) {
    listone(entry, &sb);
    return;
  }

  // It's a directory, deal with all of it
  if (S_ISDIR(sb.st_mode)) {
    // Only list the directory, not its contents
    if (dircontents==0) {
      listone(entry, &sb);
      return;
    }

    // Put the dir's name in a buffer, so that we can append
    // each filename to the buffer
    if(strlen(entry) + 1 + MAXNAMLEN + 1 > sizeof buf){
      printf("ls: path too long\n");
      return;
    }
    strcpy(buf, entry);
    p = buf+strlen(buf);
    *p++ = '/';

    // Open the directory
    D= opendir(entry);
    if (D==NULL) {
      printf("%s: unable to opendir\n", entry);
      return;
    }

    // Process each entry
    while ((dent=readdir(D))!=NULL) {

      // Skip empty directory entries
      if (dent->d_name[0]=='\0') continue;

      // Skip dot files
      if ((showdots==0) && (dent->d_name[0]=='.')) continue;

      // Append the name to the buffer
      //memmove(p, dent->d_name, MAXNAMLEN);
      memcpy(p, dent->d_name, MAXNAMLEN);

      // Get the file's stats
      if (stat(buf, &sb)==-1) {
        printf("%s: non-existent\n", dent->d_name);
        continue;
      }

      // and add the file to the array
      namelist[count].dent= dent;
      memcpy(&(namelist[count].sb), &sb, sizeof(sb));
      count++;

    }

    // Sort the array into name order
    qsort(namelist, count, sizeof(Namestat), namecmp);

    // Print each one out
    for (i=0; i < count; i++) 
      listone(namelist[i].dent->d_name, &(namelist[i].sb));

    closedir(D);
  }
}

int main(int argc, char *argv[])
{
  int opt;                      /* option letter from getopt() */
  int i;

  /* Process any command line flags. */
  while ((opt = getopt(argc, argv, "ilad")) != EOF) {
        if (opt == 'l')
                longoutput = 1;
        if (opt == 'a')
                showdots = 1;
        if (opt == 'i')
                showinums = 1;
        if (opt == 'd')
                dircontents = 0;
  }

  // No further arguments, list the current directory
  if (optind==argc) {
    listmany("."); exit(0);
  }

  // Otherwise, process the arguments left
  for (i=optind; i<argc; i++)
    listmany(argv[i]);
  exit(0);
}
