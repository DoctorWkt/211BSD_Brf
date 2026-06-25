// These come from Apout, see https://github.com/DoctorWkt/Apout
// (c) 2026 Warren Toomey, GPL3

/* fcntl defines used by open */
#define BSD_RDONLY        0x0000        /* open for reading only */
#define BSD_WRONLY        0x0001        /* open for writing only */
#define BSD_RDWR          0x0002        /* open for reading and writing */
#define BSD_NONBLOCK      0x0004        /* no delay */
#define BSD_APPEND        0x0008        /* set append mode */
#define BSD_SHLOCK        0x0010        /* open with shared file lock */
#define BSD_EXLOCK        0x0020        /* open with exclusive file lock */
#define BSD_ASYNC         0x0040        /* signal pgrp when data ready */
#define BSD_FSYNC         0x0080        /* synchronous writes */
#define BSD_CREAT         0x0200        /* create if nonexistant */
#define BSD_TRUNC         0x0400        /* truncate to zero length */
#define BSD_EXCL          0x0800        /* error if already exists */

#define copylong(to,from) \
        buf = (char *) &(to); buf2 = (char *) &(from); \
        buf[0]=buf2[2]; buf[1]=buf2[3]; buf[2]=buf2[0]; buf[3]=buf2[1]

/* stat struct, used by S_STAT, S_FSTAT, S_LSTAT */
struct tr_stat {
    int16_t st_dev;
    u_int16_t st_ino;
    u_int16_t st_mode;
    int16_t st_nlink;
    u_int16_t st_uid;
    u_int16_t st_gid;
    int16_t st_rdev;
    int8_t st_size[4];          /* Alignment problems */
    int8_t st_atim[4];          /* Alignment problems */
    int16_t st_spare1;
    int8_t st_mtim[4];          /* Alignment problems */
    int16_t st_spare2;
    int8_t st_ctim[4];          /* Alignment problems */
    int16_t st_spare3;
    int8_t st_blksize[4];       /* Alignment problems */
    int8_t st_blocks[4];        /* Alignment problems */
    u_int16_t st_flags;
    u_int16_t st_spare4[3];
};

/* Directory entry */
#define TR_DIRBLKSIZ    512
#define TR_MAXNAMLEN    63
struct tr_direct {
    u_int16_t d_ino;            /* inode number of entry */
    u_int16_t d_reclen;         /* length of this record */
    u_int16_t d_namlen;         /* length of string in d_name */
    char d_name[TR_MAXNAMLEN + 1];      /* name must be no longer than this */
};
