// Client request handling code for the Brf server.
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include "brf.h"
#include "bsdcode.h"
#include "proto.h"

// We keep an array to map server
// errno values to 2.11BSD ones
#define ERR_MAPSIZE 128
uint16_t errmap[ERR_MAPSIZE];

void build_errno_map(void) {
  int i;

  // Fill the table with something
  for (i=0; i < ERR_MAPSIZE; i++) {
    errmap[i]= BSD_EINVAL;
  }

  // Set zero entry to zero!
  errmap[i]= 0;

  // Now manually map server to 2.11BSD values
  errmap[EPERM]= BSD_EPERM;
  errmap[ENOENT]= BSD_ENOENT;
  errmap[ESRCH]= BSD_ESRCH;
  errmap[EINTR]= BSD_EINTR;
  errmap[EIO]= BSD_EIO;
  errmap[ENXIO]= BSD_ENXIO;
  errmap[E2BIG]= BSD_E2BIG;
  errmap[ENOEXEC]= BSD_ENOEXEC;
  errmap[EBADF]= BSD_EBADF;
  errmap[ECHILD]= BSD_ECHILD;
  errmap[EAGAIN]= BSD_EAGAIN;
  errmap[ENOMEM]= BSD_ENOMEM;
  errmap[EACCES]= BSD_EACCES;
  errmap[EFAULT]= BSD_EFAULT;
  errmap[ENOTBLK]= BSD_ENOTBLK;
  errmap[EBUSY]= BSD_EBUSY;
  errmap[EEXIST]= BSD_EEXIST;
  errmap[EXDEV]= BSD_EXDEV;
  errmap[ENODEV]= BSD_ENODEV;
  errmap[ENOTDIR]= BSD_ENOTDIR;
  errmap[EISDIR]= BSD_EISDIR;
  errmap[EINVAL]= BSD_EINVAL;
  errmap[ENFILE]= BSD_ENFILE;
  errmap[EMFILE]= BSD_EMFILE;
  errmap[ENOTTY]= BSD_ENOTTY;
  errmap[ETXTBSY]= BSD_ETXTBSY;
  errmap[EFBIG]= BSD_EFBIG;
  errmap[ENOSPC]= BSD_ENOSPC;
  errmap[ESPIPE]= BSD_ESPIPE;
  errmap[EROFS]= BSD_EROFS;
  errmap[EMLINK]= BSD_EMLINK;
  errmap[EPIPE]= BSD_EPIPE;
  errmap[EWOULDBLOCK]= BSD_EWOULDBLOCK;
  errmap[EDEADLK]= BSD_EDEADLK;
  errmap[EINPROGRESS]= BSD_EINPROGRESS;
  errmap[EALREADY]= BSD_EALREADY;
  errmap[ENOTSOCK]= BSD_ENOTSOCK;
  errmap[EDESTADDRREQ]= BSD_EDESTADDRREQ;
  errmap[EMSGSIZE]= BSD_EMSGSIZE;
  errmap[EPROTOTYPE]= BSD_EPROTOTYPE;
  errmap[ENOPROTOOPT]= BSD_ENOPROTOOPT;
  errmap[EPROTONOSUPPORT]= BSD_EPROTONOSUPPORT;
  errmap[ESOCKTNOSUPPORT]= BSD_ESOCKTNOSUPPORT;
  errmap[EOPNOTSUPP]= BSD_EOPNOTSUPP;
  errmap[EPFNOSUPPORT]= BSD_EPFNOSUPPORT;
  errmap[EAFNOSUPPORT]= BSD_EAFNOSUPPORT;
  errmap[EADDRINUSE]= BSD_EADDRINUSE;
  errmap[EADDRNOTAVAIL]= BSD_EADDRNOTAVAIL;
  errmap[ENETDOWN]= BSD_ENETDOWN;
  errmap[ENETUNREACH]= BSD_ENETUNREACH;
  errmap[ENETRESET]= BSD_ENETRESET;
  errmap[ECONNABORTED]= BSD_ECONNABORTED;
  errmap[ECONNRESET]= BSD_ECONNRESET;
  errmap[ENOBUFS]= BSD_ENOBUFS;
  errmap[EISCONN]= BSD_EISCONN;
  errmap[ENOTCONN]= BSD_ENOTCONN;
  errmap[ESHUTDOWN]= BSD_ESHUTDOWN;
  errmap[ETOOMANYREFS]= BSD_ETOOMANYREFS;
  errmap[ETIMEDOUT]= BSD_ETIMEDOUT;
  errmap[ECONNREFUSED]= BSD_ECONNREFUSED;
  errmap[ELOOP]= BSD_ELOOP;
  errmap[ENAMETOOLONG]= BSD_ENAMETOOLONG;
  errmap[EHOSTDOWN]= BSD_EHOSTDOWN;
  errmap[EHOSTUNREACH]= BSD_EHOSTUNREACH;
  errmap[ENOTEMPTY]= BSD_ENOTEMPTY;
  errmap[EUSERS]= BSD_EUSERS;
  errmap[EDQUOT]= BSD_EDQUOT;
  errmap[ESTALE]= BSD_ESTALE;
  errmap[EREMOTE]= BSD_EREMOTE;
  errmap[ENOLCK]= BSD_ENOLCK;
  errmap[ENOSYS]= BSD_ENOSYS;
}

// Map the server errno to 2.11BSD errno values
uint16_t map_errno() {
  if (errno >= ERR_MAPSIZE) return(BSD_EINVAL);
  return(errmap[errno]);
}

// The following two buffers are used as
// part of the translation from virtual
// absolute filenames to native ones. We
// only have 2 buffers, so if you call
// xlate_filename() 3 times, the 1st return
// value will be destroyed.
char realfilename[2][2 * MAXPATHLEN];
char *rfn[2];
int whichrfn = 0;
char *brf_root = NULL;		// Root dir for filesystem

char brf_dir[MAXPATHLEN];
int brf_dirlen;
char pathname[MAXPATHLEN];
char *databuf;

// Set the base of the simulated filesystem
void set_brf_root(char *dirname) {
  struct stat st;

  // Check the directory exists
  if ((stat(dirname, &st)!=0) || !S_ISDIR(st.st_mode)) {
    fprintf(stderr, "%s is not a directory or does not exist\n", dirname);
    exit(1);
  }

  // Copy it and make pointers to just after it
  strcpy(realfilename[0], dirname);
  strcpy(realfilename[1], dirname);
  rfn[0] = realfilename[0];
  rfn[0] += strlen(realfilename[0]);
  rfn[1] = realfilename[1];
  rfn[1] += strlen(realfilename[1]);

  databuf = (char *) malloc(65536);
}

// Translate from a 2.11BSD filename to one in the simulated local filesystem.
// Note we return a pointer to one of two buffers. The caller does not
// have to free the returned pointer, but successive calls will destroy
// calls from >2 calls earlier.
char *xlate_filename(char *name) {
  int i = whichrfn;

  name += brf_dirlen;		// Skip past the mountpoint. After this,
  				// name should point at 0 or '/'.

// printf("Translating >%s<\n", name);
  strcpy(rfn[i], name);		// Copy name into buffer
  whichrfn = 1 - whichrfn;	// Switch to other buffer next time
// printf("Translated into >%s<\n", realfilename[i]);
  return (realfilename[i]);
}

// Map the 2.11BSD fcntl mode bits to the underlying
// system's bits. We have to do this for Linux
uint16_t map_fcntl(uint16_t f) {
  int16_t out = 0;

  if (f & BSD_RDONLY)   out |= O_RDONLY;
  if (f & BSD_WRONLY)   out |= O_WRONLY;
  if (f & BSD_RDWR)     out |= O_RDWR;
  if (f & BSD_NONBLOCK) out |= O_NONBLOCK;
  if (f & BSD_APPEND)   out |= O_APPEND;
  if (f & BSD_ASYNC)    out |= O_ASYNC;
  if (f & BSD_FSYNC)    out |= O_FSYNC;
  if (f & BSD_CREAT)    out |= O_CREAT;
  if (f & BSD_TRUNC)    out |= O_TRUNC;
  if (f & BSD_EXCL)     out |= O_EXCL;

  return (out);
}

/* 2.11BSD reads directories as if they were ordinary files.
 * The solution is to read the directory entries, and build a
 * real file, which is passed back to the open call.
 *
 * A directory consists of some number of blocks of DIRBLKSIZ
 * bytes, where DIRBLKSIZ is chosen such that it can be transferred
 * to disk in a single atomic operation (e.g. 512 bytes on most machines).
 *
 * Each DIRBLKSIZ byte block contains some number of directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 *
 * The macro DIRSIZ(dp) gives the amount of space required to represent
 * a directory entry.  Free space in a directory is represented by
 * entries which have dp->d_reclen > DIRSIZ(dp).  All DIRBLKSIZ bytes
 * in a directory block are claimed by the directory entries.  This
 * usually results in the last entry in a directory having a large
 * dp->d_reclen.  When entries are deleted from a directory, the
 * space is returned to the previous entry in the same directory
 * block by increasing its dp->d_reclen.  If the first entry of
 * a directory block is free, then its dp->d_ino is set to 0.
 * Entries other than the first in a directory do not normally have
 * dp->d_ino set to 0.
 */

#define TMP_PLATE       "/tmp/tmp_dir.XXXXXX"

static int bsdopen_dir(char *name) {
  DIR *d;
  char *tmpname;
  int i, nlen, total_left;
  struct dirent *dent;
  struct tr_direct odent, empty;

  d = opendir(name);
  if (d == NULL) return (-1);
  tmpname = strdup(TMP_PLATE);
  i = mkstemp(tmpname);
  if (i == -1) {
    (void) fprintf(stderr, "opendir couldn't open %s\n", tmpname);
    exit(EXIT_FAILURE);
  }
  unlink(tmpname);
  free(tmpname);

  total_left = TR_DIRBLKSIZ;
  empty.d_ino = 0;
  empty.d_namlen = 0;
  empty.d_name[0] = '\0';
  empty.d_name[1] = '\0';
  empty.d_name[2] = '\0';
  empty.d_name[3] = '\0';

  while ((dent = readdir(d)) != NULL) {
    memset(odent.d_name, 0, TR_MAXNAMLEN + 1);	// Null name 
    nlen = strlen(dent->d_name) + 1;		// Name length
    if (nlen > TR_MAXNAMLEN + 1)
      nlen = TR_MAXNAMLEN + 1;
    strncpy(odent.d_name, dent->d_name, nlen);
    odent.d_ino = dent->d_fileno;

    // Nasty hack: ensure inode is never 0
    if (odent.d_ino == 0) odent.d_ino = 1;

    odent.d_namlen = nlen;
    nlen += (nlen & 3);			// Round up to mult of 4
    odent.d_reclen = nlen + 6;		// Name + 3 u_int16_ts

    // Not enough room, write a blank entry
    if ((total_left - odent.d_reclen) < 10) {
      empty.d_reclen = total_left;
      write(i, &empty, empty.d_reclen);
      total_left = TR_DIRBLKSIZ;
    }

    write(i, &odent, odent.d_reclen);
    total_left -= odent.d_reclen;
  }
  closedir(d);

  if (total_left) {
    empty.d_reclen = total_left;
    write(i, &empty, empty.d_reclen);
  }

  lseek(i, 0, SEEK_SET);
  return (i);
}

#define MAX_BLKSIZE	1024	/* Maximum block size from stat/fstat */

// Convert from our stat struct to 2.11BSD stat struct
void cvt_stat(struct stat *stbuf, struct tr_stat *tr_stbuf) {
  char *buf, *buf2;

  /* The following stops blksize equalling 64K,
   * which becomes 0 in a 16-bit int. This then
   * causes 2.11BSD flsbuf() to malloc(0), which
   * then causes malloc to go crazy - wkt.
   */
  if (stbuf->st_blksize > MAX_BLKSIZE)
    stbuf->st_blksize = MAX_BLKSIZE;

  tr_stbuf->st_dev =   stbuf->st_dev;
  tr_stbuf->st_ino =   stbuf->st_ino;
  tr_stbuf->st_mode =  stbuf->st_mode;
  tr_stbuf->st_nlink = stbuf->st_nlink;
  tr_stbuf->st_uid =   stbuf->st_uid;
  tr_stbuf->st_gid =   stbuf->st_gid;
  tr_stbuf->st_rdev =  stbuf->st_rdev;
  copylong(tr_stbuf->st_size, stbuf->st_size);
  copylong(tr_stbuf->st_atim, stbuf->st_atime);
  copylong(tr_stbuf->st_mtim, stbuf->st_mtime);
  copylong(tr_stbuf->st_ctim, stbuf->st_ctime);
  copylong(tr_stbuf->st_blksize, stbuf->st_blksize);
  copylong(tr_stbuf->st_blocks, stbuf->st_blocks);
}

// Server Brf client requests
void serve_requests(int fd) {
  int result;
  char *buf, *buf2;
  char *realpath;
  char *realpath2;
  struct brf_req req;
  struct brf_init binit;
  struct brf_open bopen;
  struct brf_rw brw;
  struct brf_seek bseek;
  struct brf_fchmod bchmod;
  struct brf_fchown bchown;
  struct stat st;
  struct tr_stat tr_st;
  int16_t ufd, ufd2;
  int16_t mode;
  int32_t offset;
  int cnt;
  char *dataptr;
  void *data;
  int datalen;

  while (1) {
    // Get the next request.
    if (read(fd, &req, sizeof(req)) != sizeof(req)) return;
    if (req.flag != 0xff) {
#ifdef DEBUG
      syslog(LOG_ERR, "Bad flag 0x%x", req.flag);
#endif
      return;
    }

    // Clear errno before we do anything
    errno = 0; data = NULL; datalen = 0;

    switch (req.cmd) {
    case BRF_OPINIT:
      // Get the init struct and the base directory
      result= 0;
      if (read(fd, &binit, sizeof(binit)) != sizeof(binit)) {
	result = -1; break;
      }
#ifdef DEBUG
      syslog(LOG_DEBUG, "Got init uid %d gid %d", binit.uid, binit.gid);
#endif

      req.len -= sizeof(binit);
      if (read(fd, brf_dir, req.len) != req.len) { result = -1; break; }
#ifdef DEBUG
      syslog(LOG_DEBUG, "Got brf_dir %s", brf_dir);
#endif
      brf_dirlen = strlen(brf_dir);

      // If we are root, set the uid and gid
      if (geteuid()==0) {
	setuid(binit.uid);
	setgid(binit.gid);
#ifdef DEBUG
        syslog(LOG_DEBUG, "Set uid to %d, gid to %d", binit.uid, binit.gid);
#endif
      }
      break;

    case BRF_OPOPEN:
      // Get the mode, flags and pathname;
      if (read(fd, &bopen, sizeof(bopen)) != sizeof(binit)) {
	result = -1; break;
      }

      req.len -= sizeof(bopen);
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the flags and the pathname
      bopen.flags = map_fcntl(bopen.flags);
      realpath = xlate_filename(pathname);

      // See if the pathname is a directory
      result = stat(realpath, &st);
      if (result == 0 && (st.st_mode & S_IFDIR)) {
	// Yes it is
	result = bsdopen_dir(realpath);
      } else {
	// It's a file, do the open()
	result = open(realpath, bopen.flags, bopen.mode);
      }

      // Adjust the file descriptor
      if (result != -1) result += BRF_FDOFFSET;
#ifdef DEBUG
      syslog(LOG_DEBUG, "Open on %s flags 0x%x mode 0%o returns %d",
	     pathname, bopen.flags, bopen.mode, result);
#endif
      break;

    case BRF_OPCLOSE:
      // Get the file descriptor, convert it
      if (read(fd, &ufd, sizeof(ufd)) != sizeof(ufd)) { result = -1; break; }
      ufd -= BRF_FDOFFSET;

      // Close it
#ifdef DEBUG
      syslog(LOG_DEBUG, "Close on fd %d", ufd);
#endif
      result = close(ufd);
      break;

    case BRF_OPWRITE:
      // Get the fd and nbytes
      if (read(fd, &brw, sizeof(brw)) != sizeof(brw)) { result = -1; break; }
      brw.fd -= BRF_FDOFFSET;

      // Get the data and write it to file.
      // We have to loop as we sometimes don't
      // get the amount we want
      cnt=0; result=0; dataptr= databuf;
      while (cnt != brw.nbytes) {
	result= read(fd, dataptr, brw.nbytes - cnt);
        if (result == -1) break;
	cnt += result; dataptr += result;
      }

      if (result == -1) break;
      result = write(brw.fd, databuf, brw.nbytes);
#ifdef DEBUG
      syslog(LOG_DEBUG, "Writing %d bytes to fd %d returns %d",
	     brw.nbytes, brw.fd, result);
#endif
      break;

    case BRF_OPREAD:
      // Get the fd and nbytes
      if (read(fd, &brw, sizeof(brw)) != sizeof(brw)) { result = -1; break; }
      brw.fd -= BRF_FDOFFSET;

      // Read the requested amount of data
      result = read(brw.fd, databuf, brw.nbytes);
#ifdef DEBUG
      syslog(LOG_DEBUG, "Reading %d bytes from fd %d, got %d bytes",
	     brw.nbytes, brw.fd, result);
#endif

      // Return the response to the client
      cnt = (result == -1) ? 0 : result;
      data = databuf;
      datalen = cnt;
      break;

    case BRF_OPDUP:
      // Get the file descriptor, convert it
      if (read(fd, &ufd, sizeof(ufd)) != sizeof(ufd)) { result = -1; break; }
      ufd -= BRF_FDOFFSET;

      // Do the dup(), convert the file descriptor
      result = dup(ufd);
      if (result != -1) result += BRF_FDOFFSET;
#ifdef DEBUG
      syslog(LOG_DEBUG, "dup(%d) returned %d", ufd, result);
#endif
      break;

    case BRF_OPDUP2:
      // Get the first file descriptor, convert it
      if (read(fd, &ufd, sizeof(ufd)) != sizeof(ufd)) { result = -1; break; }
      ufd -= BRF_FDOFFSET;
      // Ditto the second file descriptor
      if (read(fd, &ufd2, sizeof(ufd2)) != sizeof(ufd2)) { result = -1; break; }
      ufd2 -= BRF_FDOFFSET;

      // Do the dup2(), convert the file descriptor
      result = dup2(ufd, ufd2);
      if (result != -1) result += BRF_FDOFFSET;
#ifdef DEBUG
      syslog(LOG_DEBUG, "dup2(%d,%d) returned %d", ufd, ufd2, result);
#endif
      break;

    case BRF_OPSTAT:
      // Get the pathname
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname and do the stat()
      realpath = xlate_filename(pathname);
      if ((result = stat(realpath, &st)) == -1) { result = -1; break; }

      // Convert the stat buf into 2.11BSD format
      cvt_stat(&st, &tr_st);

      // Return the response to the client
      data = &tr_st; datalen = sizeof(tr_st);
      break;

    case BRF_OPLSTAT:
      // Get the pathname
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname and do the stat()
      realpath = xlate_filename(pathname);
      if ((result = lstat(realpath, &st)) == -1) { result = -1; break; }

      // Convert the stat buf into 2.11BSD format
      cvt_stat(&st, &tr_st);

      // Return the response to the client
      data = &tr_st; datalen = sizeof(tr_st);
      break;

    case BRF_OPFSTAT:
      // Get the file descriptor, convert it
      if (read(fd, &ufd, sizeof(ufd)) != sizeof(ufd)) { result = -1; break; }
      ufd -= BRF_FDOFFSET;

      if ((result = fstat(ufd, &st)) == -1) { result = -1; break; }

      // Convert the stat buf into 2.11BSD format
      cvt_stat(&st, &tr_st);

      // Return the response to the client
      data = &tr_st; datalen = sizeof(tr_st);
      break;

    case BRF_OPUNLINK:
      // Get the pathname
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname and do the unlink()
      realpath = xlate_filename(pathname);
      result = unlink(realpath);
      break;

    case BRF_OPLINK:
      // Get the first pathname
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // The second pathname comes after the NUL
      dataptr= pathname; dataptr+= strlen(pathname) + 1;

      // Map the pathnames and do the unlink()
      realpath = xlate_filename(pathname);
      realpath2 = xlate_filename(dataptr);
      result = link(realpath, realpath2);
      break;

    case BRF_OPLSEEK:
      // Get the struct
      if (read(fd, &bseek, sizeof(bseek)) != sizeof(bseek)) {
	result = -1; break;
      }
      bseek.fd -= BRF_FDOFFSET;

      // Convert the offset
      copylong(offset, bseek.offset);

      // Do the lseek()
      offset= lseek(bseek.fd, offset, bseek.whence);

      // Convert the offset back
      copylong(bseek.offset, offset);
      result= offset; data= &bseek; datalen= sizeof(bseek);

      break;

    case BRF_OPRENAME:
      // Get the first pathname
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // The second pathname comes after the NUL
      dataptr= pathname; dataptr+= strlen(pathname) + 1;

      // Map the pathnames and do the rename()
      realpath = xlate_filename(pathname);
      realpath2 = xlate_filename(dataptr);
      result = rename(realpath, realpath2);
      break;

    case BRF_OPACCESS:
      // Get the mode and pathname;
      if (read(fd, &mode, sizeof(mode)) != sizeof(mode)) {
	result = -1; break;
      }

      req.len -= sizeof(mode);
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname
      realpath = xlate_filename(pathname);
      result= access(realpath, mode);
      break;

    case BRF_OPMKDIR:
      // Get the mode and pathname;
      if (read(fd, &mode, sizeof(mode)) != sizeof(mode)) {
	result = -1; break;
      }

      req.len -= sizeof(mode);
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname
      realpath = xlate_filename(pathname);
      result= mkdir(realpath, mode);
      break;

    case BRF_OPRMDIR:
      // Get the pathname;
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname
      realpath = xlate_filename(pathname);
      result= rmdir(realpath);
      break;

    case BRF_OPCHMOD:
      // Get the mode and pathname;
      if (read(fd, &mode, sizeof(mode)) != sizeof(mode)) {
	result = -1; break;
      }

      req.len -= sizeof(mode);
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname
      realpath = xlate_filename(pathname);
      result= chmod(realpath, mode);
      break;

    case BRF_OPFCHMOD:
      // Get the file descriptor and mode
      if (read(fd, &bchmod, sizeof(bchmod)) != sizeof(bchmod)) {
	result = -1; break;
      }

      bchmod.fd -= BRF_FDOFFSET;
      result= fchmod(bchmod.fd, bchmod.mode);
      break;

    case BRF_OPCHOWN:
      // Get the init struct as it has uid and gid
      if (read(fd, &binit, sizeof(binit)) != sizeof(binit)) {
        result = -1; break;
      }

      // Get the pathname
      req.len -= sizeof(binit);
      if (read(fd, pathname, req.len) != req.len) { result = -1; break; }

      // Map the pathname
      realpath = xlate_filename(pathname);
      result= chown(realpath, binit.uid, binit.gid);
      break;

    case BRF_OPFCHOWN:
      // Get the file descriptor, uid and gid
      if (read(fd, &bchown, sizeof(bchown)) != sizeof(bchown)) {
	result = -1; break;
      }

      bchown.fd -= BRF_FDOFFSET;
      result= fchown(bchown.fd, bchown.uid, bchown.gid);
      break;

    default:
#ifdef DEBUG
      syslog(LOG_ERR, "Unknown command %d", req.cmd);
#endif
      return;
    }

    // Send the response back to the client
    brf_respond(fd, result, req.cmd, data, datalen);
  }

  // No more requests
#ifdef DEBUG
  syslog(LOG_DEBUG, "Client disconnected on fd %d", fd);
#endif
  close(fd);
}
