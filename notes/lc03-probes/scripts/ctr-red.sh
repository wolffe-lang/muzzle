#!/usr/bin/env bash
# lc03, in the container at /work: the red run (every case through
# tools/difftest on both tiers; the new ones cannot build their muzzle
# column yet), glibc's outputs kept as the prediction's expected output,
# and the uapi numbers the implementation needs (allowed paths, numbers
# only, each file's package checked with dpkg -S before it is read).
set -u
cd /work
step() { local n=$1; shift; echo "BEGIN $n"; "$@"; local rc=$?; echo "END $n exit=$rc"; }
facts() { cat /work/.lc03-sha; clang --version | head -1; getconf GNU_LIBC_VERSION; uname -srm; }
step env facts
step fetch env WOLF_ARCHIVE=/archives/wolf-0.2.36-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain
keep() {
  local t=$1 d=/out/expected-$1
  mkdir -p "$d"
  for f in out/$t/*.glibc.out out/$t/*.glibc.err; do cp "$f" "$d/"; done
  (cd "$d" && sha256sum * >SHA256SUMS)
  echo "kept $(ls "$d" | wc -l) files in $d"
}
for t in native release; do
  step "build.$t" tools/build $t
  step "difftest.$t" tools/difftest $t ${CASES:-tests/cases/*.c tests/next/*.c}
  step "keep.$t" keep $t
done
uapi() {
  local f=$1; shift
  local owner; owner=$(dpkg -S "$f" 2>/dev/null | cut -d: -f1)
  echo "FILE $f owner=${owner:-none}"
  [ "$owner" = linux-libc-dev ] || { echo "  not read: not linux-libc-dev"; return 1; }
  if [ $# = 0 ]; then
    grep -E '^#define[[:space:]]+E[A-Z0-9]+[[:space:]]' "$f" | awk '{print "  " $2, $3}'
  else
    for name in "$@"; do grep -E "^#define[[:space:]]+$name[[:space:]]" "$f" | head -1 | awk '{print "  " $2, $3}'; done
  fi
}
numbers() {
  uapi /usr/include/asm-generic/errno-base.h
  uapi /usr/include/asm-generic/errno.h
  uapi /usr/include/x86_64-linux-gnu/asm/unistd_64.h __NR_stat __NR_fstat __NR_lstat __NR_rt_sigaction \
    __NR_rt_sigprocmask __NR_access __NR_pipe __NR_dup __NR_dup2 __NR_nanosleep __NR_getpid __NR_kill __NR_getcwd \
    __NR_chdir __NR_rename __NR_mkdir __NR_rmdir __NR_unlink __NR_symlink __NR_readlink __NR_gettid __NR_time \
    __NR_getdents64 __NR_clock_gettime __NR_tgkill __NR_openat __NR_newfstatat __NR_faccessat __NR_dup3 __NR_pipe2
  uapi /usr/include/linux/time.h CLOCK_REALTIME CLOCK_MONOTONIC CLOCK_PROCESS_CPUTIME_ID CLOCK_THREAD_CPUTIME_ID
  uapi /usr/include/linux/fcntl.h AT_FDCWD AT_SYMLINK_NOFOLLOW AT_REMOVEDIR AT_EACCESS AT_SYMLINK_FOLLOW AT_EMPTY_PATH
  uapi /usr/include/linux/stat.h S_IFMT S_IFSOCK S_IFLNK S_IFREG S_IFBLK S_IFDIR S_IFCHR S_IFIFO S_ISUID S_ISGID \
    S_ISVTX S_IRWXU S_IRUSR S_IWUSR S_IXUSR S_IRWXG S_IRGRP S_IWGRP S_IXGRP S_IRWXO S_IROTH S_IWOTH S_IXOTH
  uapi /usr/include/x86_64-linux-gnu/asm/signal.h SIGABRT SIG_BLOCK SIG_UNBLOCK SIG_SETMASK
  uapi /usr/include/asm-generic/signal-defs.h SIG_BLOCK SIG_UNBLOCK SIG_SETMASK SIG_DFL SIG_IGN
  local f=/usr/include/x86_64-linux-gnu/asm/stat.h owner
  owner=$(dpkg -S "$f" 2>/dev/null | cut -d: -f1)
  echo "FILE $f owner=${owner:-none} (x86-64 struct stat, from its line to the closing brace)"
  if [ "$owner" = linux-libc-dev ]; then
    awk '/^#(if|else|endif|ifdef)/{print "  " $0} /^struct stat \{/{on=1; n++; print "  [" n "]"} on{print "  " $0} on && /^\};/{on=0}' "$f"
  fi
  f=/usr/include/x86_64-linux-gnu/asm/signal.h
  owner=$(dpkg -S "$f" 2>/dev/null | cut -d: -f1)
  echo "FILE $f owner=${owner:-none} (struct sigaction)"
  [ "$owner" = linux-libc-dev ] && awk '/^struct sigaction \{/{on=1} on{print "  " $0} on && /^\};/{on=0}' "$f"
}
step uapi numbers
exit 0
