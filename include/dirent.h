/* muzzle: <dirent.h> (POSIX.1-2024 XBD dirent.h). Hand-written: struct
 * dirent is the record getdents64(2) writes (struct linux_dirent64), so
 * readdir answers the kernel's record as it is; d_type's values are
 * S_IFMT's bits shifted down 12 (uapi linux/stat.h). DIR is opaque. */
#ifndef _MUZZLE_DIRENT_H
#define _MUZZLE_DIRENT_H

#include <sys/types.h>

typedef struct __muzzle_dir DIR;

struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12

DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif
