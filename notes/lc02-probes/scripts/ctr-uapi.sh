#!/usr/bin/env bash
# lc02: the uapi numbers muzzle's headers and sources use, read from the
# Linux uapi headers (allowed: numbers and constants only). Each file's
# package is checked first with dpkg -S, and only a linux-libc-dev file
# is read; only the named #define lines are printed.
set -u
check() {
  local f=$1; shift
  local owner; owner=$(dpkg -S "$f" 2>/dev/null | cut -d: -f1)
  echo "FILE $f owner=${owner:-none}"
  [ "$owner" = linux-libc-dev ] || { echo "  not read: not linux-libc-dev"; return 1; }
  for name in "$@"; do
    grep -E "^#define[[:space:]]+$name[[:space:]]" "$f" | head -1 | sed 's/^/  /'
  done
}
check /usr/include/x86_64-linux-gnu/asm/unistd_64.h __NR_read __NR_write __NR_close __NR_fstat __NR_lseek __NR_ioctl __NR_fcntl __NR_openat
check /usr/include/asm-generic/fcntl.h O_ACCMODE O_RDONLY O_WRONLY O_RDWR O_CREAT O_EXCL O_NOCTTY O_TRUNC O_APPEND O_NONBLOCK O_DSYNC O_DIRECTORY O_NOFOLLOW O_CLOEXEC __O_SYNC O_SYNC __O_TMPFILE O_TMPFILE F_GETFL
check /usr/include/linux/fcntl.h AT_FDCWD
check /usr/include/asm-generic/ioctls.h TCGETS
check /usr/include/asm-generic/errno.h EOVERFLOW EILSEQ
f=/usr/include/x86_64-linux-gnu/asm/stat.h
owner=$(dpkg -S "$f" 2>/dev/null | cut -d: -f1)
echo "FILE $f owner=${owner:-none}"
if [ "$owner" = linux-libc-dev ]; then
  # every struct stat block (the file holds i386's and x86-64's), from its
  # first line to st_blksize, numbered, with the preprocessor lines between
  awk '/^#(if|else|endif)/{print "  " $0} /^struct stat \{/{on=1; n++; print "  [" n "]"} on{print "  " $0} on && /st_blksize/{on=0}' "$f"
fi
exit 0
