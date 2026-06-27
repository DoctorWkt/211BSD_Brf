// Utility functions for Brf
// (c) 2026 Warren Toomey, GPL3

#include <sys/types.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include "brf.h"

// File descriptor for the server connection.
// Also: -1 means connection failed,
//       -2 means no connection as yet
static int brf_fd = -2;

// Buffer that holds the directory name which is remote
// and the length of the directory name
#define BUFSIZE 80
static char brf_dir[BUFSIZE];
static int brf_dirlen;

// Given a brf command, pointer to any following data and the
// datas' length, send the request to the server. Return 0 on
// success or -1 on failure
int brf_send(int cmd, void *data1, int data1len,
		      void *data2, int data2len) {
  struct brf_req ref;

  ref.flag= 0xff;		// Build the command
  ref.cmd= cmd;
  ref.len= data1len + data2len;

				// Send the command
#ifdef EXTRA_DEBUG
  syslog(LOG_DEBUG, "brf_conn: Sending cmd %d", cmd);
#endif
  if (send(brf_fd, &ref, sizeof(ref), 0) < 0) goto bad;

				// Send any extra data
  if (data1len>0) {
#ifdef EXTRA_DEBUG
    syslog(LOG_DEBUG, "brf_conn: Sending data1len %d\n", data1len);
#endif
    if (send(brf_fd, data1, data1len, 0) < 0) goto bad;
  }

  if (data2len>0) {
#ifdef EXTRA_DEBUG
    syslog(LOG_DEBUG, "brf_conn: Sending data2len %d\n", data2len);
#endif
    if (send(brf_fd, data2, data2len, 0) < 0) goto bad;
  }

  return(0);

bad:
#ifdef DEBUG
    syslog(LOG_DEBUG, "brf_send: Failed");
#endif
  sysclose(brf_fd);
  brf_fd= -1;		// Mark a failed connection
  return(-1);
}

// Receive a server response including any data.
// Return the result value. Set errno if required.
int brf_recv(void *data, int *datalen) {
  struct brf_resp bresp;
  int num;
  uint16_t limit;

  // Receive the response struct. I thought recv() would block
  // until there is data, but sometimes it returns 0. The loop
  // ensures that there is either data or a dropped TCP connection.
  while (1) {
    num= recv(brf_fd, &bresp, sizeof(bresp), 0);
    if (num != 0) break;
#ifdef DEBUG
    syslog(LOG_DEBUG, "brf_recv: looping");
#endif
  }

  if (num != sizeof(bresp)) {
#ifdef DEBUG
    syslog(LOG_DEBUG, "brf_recv: response sized %d not sized %d",
		num, sizeof(bresp));
#endif
    goto bad;
  }

  // Check the flag
  if (bresp.flag != 0xff) {
#ifdef DEBUG
    syslog(LOG_DEBUG, "brf_recv: response flag not 0xff");
#endif
    goto bad;
}

#ifdef EXTRA_DEBUG
    syslog(LOG_DEBUG, "brf_recv: received result %d errno %d len %d",
		bresp.result, bresp.err, bresp.len);
#endif

#define MAX_RECVSIZE 1400

  // Get any extra data. It seems that large recv()s
  // fail, so we take bites at the input until we get it all.
  if (bresp.len > 0) {
    *datalen= bresp.len;
    while (bresp.len != 0) {
      limit= (bresp.len > MAX_RECVSIZE) ? MAX_RECVSIZE : bresp.len;
      if ((num= recv(brf_fd, data, limit, 0)) < 0) goto bad;
      bresp.len -= num; data += num;
    }
  }

  // Set errno if required.
  if (bresp.err != 0) errno= bresp.err;
  return(bresp.result);

bad:
#ifdef DEBUG
    syslog(LOG_DEBUG, "brf_recv: Failed");
#endif
  sysclose(brf_fd);
  brf_fd= -1;		// Mark a failed connection
  return(-1);
}

// Parse the configuration file and open a connection
// to the server. Return 0 if OK, -1 on failure.
static int brf_conn() {
  int zin;
  char *servername;
  char *serverport;
  int sock;
  struct hostent *hp;
  struct sockaddr_in server;
  struct brf_init binit;

#ifdef DEBUG
  // Connect to the system logger
  openlog("brf", LOG_PID, LOG_USER);
#endif

  // Open the configuration file
  if ((zin= sysopen("/etc/brf.conf", O_RDONLY))==-1) goto bad;

  // Read and parse the contents
  if (sysread(zin, brf_dir, BUFSIZE) <=0) { sysclose(zin); goto bad; }
  sysclose(zin);

  // Find, zero and skip two colons
  if ((servername=strchr(brf_dir, ':'))==NULL) goto bad;
  *servername=0; ++servername;
  if ((serverport=strchr(servername, ':'))==NULL) goto bad;
  *serverport=0; ++serverport;
  brf_dirlen= strlen(brf_dir);

#ifdef DEBUG
  syslog(LOG_DEBUG, "brf_conn: dir %s server %s port %s",
	brf_dir, servername, serverport);
#endif

  // Try to open the connection to the server
  if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0) goto bad;
  server.sin_family = AF_INET;
  if ((hp = gethostbyname(servername))==NULL) goto bad;

  bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
  server.sin_port = htons(atoi(serverport));
  if (connect(sock, &server, sizeof(server)) < 0) goto bad;
  brf_fd= sock;

  // Send the initialisation command with uid, gid and brf_dir
  binit.uid= getuid();
  binit.gid= getgid();
  if (brf_send(BRF_OPINIT, &binit, sizeof(struct brf_init),
		brf_dir, brf_dirlen+1)<0) goto bad;

  // Now get the response
  if (brf_recv(NULL, NULL)<0) goto bad;

good:
  return(0);

bad:
#ifdef DEBUG
  syslog(LOG_DEBUG, "brf_conn: could not connect to the server");
#endif
  brf_fd= -1;
  return(-1);
}

// Given a pathname, return either the file descriptor
// to the remote server TCP connection (if the file is
// remote) or -1 (if the file is local or no server connection).
int brf_fname(char *fname) {
  // Server connection failed, return
  if (brf_fd == -1) return(-1);

  // Filename is not absolute, return
  if ((fname == NULL) || (*fname != '/')) return(-1);

  // Open a connection to the server if needed
  if ((brf_fd == -2) && (brf_conn() == -1)) return(-1);

  // If the absolute filename matches the remote directory,
  // then return the file descriptor, else return -1.
  // Ensure that the character after brf_dir is either NUL or '/'.
  // This ensures that we don't match, e.g. "/foot" when brf_dir=="/foo".
  if (!strncmp(brf_dir, fname, brf_dirlen) &&
      (fname[brf_dirlen]==0 || fname[brf_dirlen]=='/')) return(brf_fd);
  else return(-1);
}
