// Definitions for Brf
// (c) 2026 Warren Toomey, GPL3

// Brf commands
#define BRF_OPINIT	 1	// Initialise the connection
#define BRF_OPOPEN	 2	// open()
#define BRF_OPCLOSE	 3	// close()
#define BRF_OPREAD	 4	// read()
#define BRF_OPWRITE	 5	// write()
#define BRF_OPDUP	 6	// dup()
#define BRF_OPDUP2	 7	// dup2()
#define BRF_OPSTAT	 8	// stat()
#define BRF_OPLSTAT	 9	// lstat()
#define BRF_OPFSTAT	10	// fstat()
#define BRF_OPUNLINK	11	// unlink()
#define BRF_OPLINK	12	// link()
#define BRF_OPLSEEK	13	// lseek()
#define BRF_OPRENAME	14	// rename()
#define BRF_OPACCESS	15	// access()
#define BRF_OPMKDIR	16	// mkdir()
#define BRF_OPRMDIR	17	// rmdir()
#define BRF_OPCHMOD	18	// chmod()
#define BRF_OPFCHMOD	19	// fchmod()
#define BRF_OPTRUNCATE	20	// truncate()
#define BRF_OPFTRUNCATE	21	// ftruncate()
#define BRF_OPCREAT	22	// creat()
#define BRF_OPCHOWN	23	// chown()
#define BRF_OPFCHOWN	24	// fchown()

// Brf file descriptor offset
#define BRF_FDOFFSET	100

// Messages exchanged between client and server

struct brf_req {	// Client request
  uint8_t  flag;	// Must be 0xFF
  uint8_t  cmd;		// Command
  uint16_t len;		// Length of any following data
};

struct brf_resp {	// Server response
  uint8_t  flag;	// Must be 0xFF
  uint8_t  cmd;		// Command, echoed back
  int16_t   result;	// Result of operation
  uint16_t  err;	// Errno value
  uint16_t len;		// Length of any following data
};

struct brf_init {       // BRF_OPINIT data
  uint16_t  uid;        // uid of process
  uint16_t  gid;        // gid of process
                        // then: brf_dir NUL terminated
};

struct brf_rw {		// Read/write client request
  int16_t fd;		// File descriptor
  uint16_t nbytes;	// Number of bytes
};

struct brf_seek {	// lseek() client request
  int32_t offset;	
  int16_t fd;
  int16_t whence;
};

// Prototypes

int sysopen(char *path, int flags, ...);
int sysclose(int fd);
int sysread(int fd, char *buf, int nbytes);
int syswrite(int fd, char *buf, int nbytes);
int sysdup(int fd);
int sysdup2(int fd, int fd2);
int sysstat(char *path, struct stat *buf);
int syslstat(char *path, struct stat *buf);
int sysfstat(int fd, struct stat *buf);
int syslink(char *path, char *path2);
int sysunlink(char *path);
off_t syslseek(int, off_t, int);
int sysrename(char *path, char *path2);
int sysaccess(char *path, int mode);
int sysmkdir(char *path, int mode);
int sysrmdir(char *path);
int syschmod(char *path, int mode);
int sysfchmod(int fd, int mode);
int syschown(char *path, uid_t, gid_t);
int sysfchown(int, uid_t, gid_t);

int brf_send(int cmd, void *data1, int data1len,
                      void *data2, int data2len);
int brf_recv(void *data, int *datalen);
int brf_fname(char *fname);
