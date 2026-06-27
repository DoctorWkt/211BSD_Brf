// Parse /etc/brf.conf and build
// a binary version of its contents
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

// Buffer that holds the directory name which is remote
// and the length of the directory name
#define BUFSIZE 80
static char brf_dir[BUFSIZE];
static int brf_dirlen;

// Parse the configuration file and open a connection
// to the server. Return 0 if OK, -1 on failure.
int main() {
  int zin, zout;
  char *servername;
  char *serverport;
  int sock;
  struct hostent *hp;
  struct sockaddr_in server;

  // Open the text configuration file
  if ((zin= open("/etc/brf.conf", O_RDONLY))==-1) {
    perror("Could not open /etc/brf.conf"); exit(1);
  }

  // Open the binary configuration file for writing
  umask(022);
  if ((zout= open("/etc/brf.bcnf", O_WRONLY|O_CREAT, 0644))==-1) {
    perror("Could not open /etc/brf.bcnf"); exit(1);
  }

  // Read and parse the text contents
  if (read(zin, brf_dir, BUFSIZE) <=0) { 
    perror("Could not read /etc/brf.conf"); exit(1);
  }
  close(zin);

  // Find, zero and skip two colons
  if ((servername=strchr(brf_dir, ':'))==NULL) {
    perror("/etc/brf.conf badly formatted, no colon"); exit(1);
  }
  *servername=0; ++servername;

  if ((serverport=strchr(servername, ':'))==NULL) {
    perror("/etc/brf.conf badly formatted, no colon"); exit(1);
  }
  *serverport=0; ++serverport;

  brf_dirlen= strlen(brf_dir)+1;	// Include the NUL

  // Create the server structure
  server.sin_family = AF_INET;
  if ((hp = gethostbyname(servername))==NULL) {
    perror("Server name in /etc/brf.conf unknown"); exit(1);
  }
  bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
  server.sin_port = htons(atoi(serverport));

  // Write out the server struct, the length of brf_dir (incl. NUL)
  // and the brf_dir itself
  write(zout, &server, sizeof(server));
  write(zout, &brf_dirlen, sizeof(brf_dirlen));
  write(zout, brf_dir, brf_dirlen);
  close(zout);
  exit(0);
}
