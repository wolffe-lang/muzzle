# muzzle

The wolf C library: a C17/POSIX libc for x86-64 Linux, written in
[wolf](https://github.com/wolffe-lang/wolf-lang). Named for the part of a
wolf that meets the world, and for the libc it is not.
muzzle talks to the kernel only through the Linux x86-64 syscall ABI,
so a program linked against it statically is, unchanged, a userspace
test for PAX.

**State: stdlib, errno, time and the POSIX file layer (lc03), over stdio
(lc02) and the first layer (lc01).** Raw syscalls, errno, the memory and string core,
the allocator and process exit (lc01); streams with the three buffering
modes, the descriptor calls under them, and the printf family over wolf
0.2.36's C variadics (lc02); aligned allocation, sorting and searching,
the integer conversions and a correctly rounded `strtod`, the
environment, `atexit` and `abort`, `rand`, and every errno Linux defines
with glibc's `strerror` text and `perror`; `<time.h>` with the local zone
taken as UTC, and the POSIX file layer with directory streams (lc03).
Three small programs in plain C (`wc`, `sort` and a tree lister, in
`tests/cases/prog_*.c`) build against muzzle and run as they do against
glibc. A C program built with
`clang -nostdlib` runs against muzzle alone. The floating conversions of
printf (`%f %e %g %a`) are not here yet: a call that meets one answers
-1 with errno `EINVAL`, and a stream receives none of its output (a later
layer provides them; `docs/LEDGER.md`).

## What C gets

| header | functions |
|---|---|
| `<stdio.h>` | `fopen` `fdopen` `fclose` `fflush` `setvbuf` `setbuf` `fileno` `fread` `fwrite` `fgetc` `getc` `getchar` `fgets` `ungetc` `fputc` `putc` `putchar` `fputs` `puts` `feof` `ferror` `clearerr`; `perror` `rename`; `printf` `fprintf` `sprintf` `snprintf` `vprintf` `vfprintf` `vsprintf` `vsnprintf`; `stdin` `stdout` `stderr` |
| `<string.h>` | `memcpy` `memmove` `memset` `memcmp` `strlen` `strcmp` `strncmp` `strchr` `strcpy` `strerror` |
| `<stdlib.h>` | `malloc` `free` `calloc` `realloc` `aligned_alloc` `posix_memalign`; `qsort` `bsearch`; `strtol` `strtoll` `strtoul` `strtoull` `strtod` `atoi` `atol` `atoll`; `abs` `labs` `llabs` `div` `ldiv` `lldiv`; `rand` `srand`; `getenv` `setenv` `unsetenv`; `atexit` `exit` `_Exit` `abort` |
| `<unistd.h>` | `read` `write` `lseek` `close` `isatty` `_exit` `access` `chdir` `getcwd` `dup` `dup2` `pipe` `rmdir` `unlink` `symlink` `readlink` |
| `<sys/stat.h>` | `stat` `fstat` `lstat` `mkdir` and struct stat |
| `<dirent.h>` | `opendir` `readdir` `closedir` |
| `<time.h>` | `time` `clock` `clock_gettime` `nanosleep` `gmtime` `gmtime_r` `localtime` `localtime_r` `mktime` `timegm` `strftime` |
| `<fcntl.h>` | `open` `openat` and their flags |
| `<errno.h>` | `errno` (`(*__errno_location())`) and every number Linux defines (133 names, and `ENOTSUP`) |
| `<stdarg.h>` `<stddef.h>` `<stdint.h>` `<limits.h>` `<sys/types.h>` `<wchar.h>` | the types and macros (`<wchar.h>`: its types only) |

printf's conversions are `d i u o x X c s p n %` with every flag
(`- + space # 0`, and `'`), width and precision (digits or `*`), and the
lengths `hh h l ll j z t`; `%lc`, `%ls`, `%C` and `%S` in the C locale.

```sh
clang -std=c17 -fno-builtin -fno-stack-protector \
      -nostdinc -isystem muzzle/include -nostdlib -static \
      build/native/crt0.o prog.c build/native/libmuzzle.a -o prog
```

`crt0.o` holds `_start`, which stores `envp` in `environ`, calls
`main(argc, argv, envp)` and then `exit`, which runs the `atexit`
handlers and flushes every open stream. `-fno-stack-protector` is
needed until muzzle sets up TLS (`docs/LEDGER.md`).

## Two design rules

1. **muzzle never links `libwolf_rt.a`.** wolf's runtime is a hosted
   library over glibc. muzzle builds for the freestanding target
   (`x86_64-unknown-none`), so it is written only in what needs no
   runtime: scalars, raw pointers, `#[repr(c)]` data, the raw tier. No
   `List`, no `str` allocation, no region, no task. `tools/build`
   refuses an archive with any undefined symbol.
2. **muzzle is native and release only.** The checked machine and lupin
   have no C membrane, no assembly and no freestanding target. muzzle's
   oracle is the glibc differential, run against both compiling tiers.

## Layout

| path | what |
|---|---|
| `src/muzzle.lu` | the package root |
| `src/sys/` | the kernel boundary: syscalls, the error convention, errno, the trap hook |
| `src/string/` | `<string.h>` |
| `src/heap/` | the allocator, over `brk` and `mmap` |
| `src/process/` | `exit`, `_Exit`, `_exit`, `atexit`, `abort`, `write` |
| `src/stdlib/` | sorting and searching, the integer conversions, `strtod` (in integers), arithmetic and `rand`, the environment |
| `src/errtext/` | `strerror`, over `errtext.S`'s table |
| `src/unistd/` | `open`, `close`, `read`, `lseek`, `isatty`, and what stdio asks a descriptor; the POSIX file layer (`fs.lu`) and directory streams (`dirent.lu`) |
| `src/chrono/` | `<time.h>`: the clocks (`clock.lu`), the calendar (`civil.lu`), `strftime` |
| `src/stdio/` | streams (`file.lu`), character and direct I/O (`char.lu`), the printf family (`format.lu`), `perror` |
| `src/sys.S` | the one syscall routine, two trampolines into C function pointers, `strtod`'s entry, and the static storage wolf cannot hold yet: errno, `environ`, the allocator's bins, the standard streams and `stdin`/`stdout`/`stderr`, the first 32 atexit handlers, strtod's big integers |
| `src/errtext.S` | strerror's texts: glibc's, as the differential observed them (`notes/lc03-probes/gen-errtext.py`) |
| `src/crt0.S` | `_start` |
| `include/` | the C headers, hand-written from the standards' synopses (muzzle's one permanent C) |
| `tests/cases/` | the differential's cases, one per function family (a `.in` file beside a case is its stdin) |
| `tests/own/` | cases where a decision makes muzzle's behaviour its own (the floating conversions refused): self-checking, run against muzzle alone |
| `tests/lib/` | `t.h` (the cases' write(2) output helper) and `ptyrun.c` (a pseudo-terminal for the `tty` cases) |
| `tests/difftest-selftest` | each comparison `tools/difftest` makes, planted once and seen red by name (`tests/selftest/`) |
| `tests/stale-compiler` | `tools/build` runs the pinned compiler or refuses: a stale one is staged on purpose (muzzle#4) |
| `tools/` | `fetch-toolchain`, `build`, `difftest` |
| `docs/SOURCES.md` | every source consulted (the clean-room record) |
| `docs/LEDGER.md` | open behaviours the cases observe, and the link's departures |
| `notes/` | lane notes, probes and evidence logs |

## Build and test

On an x86-64 Linux host with clang:

```sh
tools/fetch-toolchain        # wolf 0.2.36 by digest (wolf-toolchain.toml)
tools/build native           # or release; gates the archive's symbols
tests/difftest-selftest native   # the differential can say no
tools/difftest native        # every case against glibc; non-zero on any difference
```

`tools/difftest` compiles each case twice: against the host glibc, a
black box, and statically against muzzle alone. It runs each column in
a fresh directory of its own, with `TZ=UTC` (muzzle's local zone is UTC)
and no core files, and compares, byte for byte, stdout,
stderr, the exit status and every file the case wrote. A case reads its
stdin from its `.in` file, and a case marked `// difftest: tty` runs
with its stdout on a pseudo-terminal. A case that fails to build in
either column is a failure, and so is a run where no case ran. CI
(`.github/workflows/ci.yml`) runs both tiers on ubuntu x86-64.

## Clean room

No C library's source is read here: not glibc, musl, the BSD libcs or
any other. muzzle is written from C17, POSIX.1-2024, the man-pages
project, the x86-64 psABI and the Linux uapi tables. `docs/SOURCES.md`
records every source consulted.

## Licence

muzzle is **GPL-3.0-or-later** (see `LICENSE`) **with the muzzle Library
Exception** (see `LICENSE-EXCEPTION`): a program you link against muzzle,
statically or dynamically, is yours under any license; changes to muzzle
itself stay under the GPL. The exception is modeled on the wolf Runtime
Library Exception and the GCC Runtime Library Exception. The wolf Training Data
Permission (see `LICENSE-TRAINING-DATA`) lets you train models on this
repository's text and ship excerpts of it in datasets under CC BY 4.0.
