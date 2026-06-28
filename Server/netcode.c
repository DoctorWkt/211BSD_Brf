// Network side code for the Brf server.
// (c) 2026 Warren Toomey, GPL3

#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/uio.h>
#include "brf.h"
#include "proto.h"

#define PORT 1170		// We listen on this port for client requests

int server_fd = -2;		// Socket file descriptor, not yet connected
int foreground = 0;		// Run in the foreground

// Reap any children that have terminated
void handle_sigchld(int sig) {

  // Stash the current errno value
  int saved_errno = errno;

  // Wait for any children, stop looping
  // when none left
  while (waitpid(-1, NULL, WNOHANG) > 0) {
  }

  // Restore errno
  errno = saved_errno;
}

// Bind to a TCP port
void bind_server_port(void) {
  struct sockaddr_in address;
  int opt = 1;

#ifdef DEBUG
  openlog("brfserver", LOG_PID, LOG_USER);
#endif

  // Make and bind a socket if not already done
  if (server_fd == -2) {
    // Create the socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      perror("Unable to create socket");
      exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // Enable SO_REUSEADDR so we can bind immediately after
    // a previous server has exited
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
		   &opt, sizeof(opt)) < 0) {
      perror("setsockopt SO_REUSEADDR failed");
      exit(1);
    }

    // Now bind to the TCP port
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
      perror("Unable to bind to TCP port");
      exit(1);
    }
  }

#ifdef DEBUG
  syslog(LOG_DEBUG, "Listening on port %d\n", PORT);
#endif
  if (listen(server_fd, 10) < 0) {
    perror("Unable to listen on TCP port");
    exit(1);
  }
}

// Accept a new client connection
int get_client_conn(void) {
  int new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
			   (socklen_t *) & addrlen)) < 0) {
#ifdef DEBUG
    syslog(LOG_ERR, "Unable to accept connection");
#endif
    exit(1);
  }

  return (new_socket);
}

// Respond to a client's request with the result.
// Return 0 if OK, -1 on error.
int brf_respond(int fd, int result, int cmd, void *data, int datalen) {
  struct brf_resp bresp;
  int err;
  struct iovec iov[2];
  int iovcnt;

  // Fill in the response fields
  bresp.flag = 0xff;
  bresp.cmd = cmd;
  bresp.result = result;
  bresp.len = datalen;
  bresp.err = map_errno();

  // Build the iovec table
  iov[0].iov_base= &bresp;
  iov[0].iov_len= sizeof(bresp);
  iov[1].iov_base= data;
  iov[1].iov_len= datalen;
  iovcnt= (datalen != 0) ? 2 : 1;
  

  // Send the response
  err = writev(fd, iov, iovcnt);
  if (err != (iov[0].iov_len + iov[1].iov_len)) return (-1);

  return (0);
}

void usage(char *name) {
  fprintf(stderr, "Usage: %s [-f] basedir\n", name); exit(1);
}

int main(int argc, char *argv[]) {
  int fd;
  int opt;
  struct sigaction sa;

  // Get any command-line flag
  while ((opt = getopt(argc, argv, "f")) != -1) {
    switch (opt) {
    case 'f':
      foreground = 1; break;
    default:
      usage(argv[0]);
    }
  }

  if (optind >= argc) usage(argv[0]);

  // Set the base of the simulated filesystem
  // and build the errno map
  set_brf_root(argv[optind]);
  build_errno_map();

  // Bind to our TCP port
  bind_server_port();

  // Become a daemon if the foreground flag is not set
  if (!foreground) daemon(1, 0);

  // Set up the sigaction structure to catch exited children
  sa.sa_handler = &handle_sigchld;
  sigemptyset(&sa.sa_mask);

  // SA_RESTART automatically restarts interrupted system calls
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

  // Register the signal handler
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction failed");
    exit(1);
  }

  while (1) {
    // Get a connection
    fd = get_client_conn();
#ifdef DEBUG
    syslog(LOG_DEBUG, "Client connected on fd %d", fd);
#endif

    // If we are running in the foreground, handle the
    // client ourselves. If in the background, fork()
    // and let the child handle the requests.
    if (foreground) {
      serve_requests(fd);
    } else {
      switch (fork()) {
      case 0:
	serve_requests(fd); exit(0);
      }
    }
  }

  return (0);
}
