// bsdcode.c
void set_brf_root(char *dirname);
char *xlate_filename(char *name);
uint16_t map_fcntl(uint16_t f);
void serve_requests(int fd);

// netcode.c
void handle_sigchld(int sig);
void bind_server_port(void);
int get_client_conn(void);
int brf_respond(int fd, int result, int cmd, void *data, int datalen);
void usage(char *name);
int main(int argc, char *argv[]);
