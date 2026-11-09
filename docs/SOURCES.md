# Sources

muzzle is written clean-room (the libc track's rule): **no C library's
source is read** (not glibc, musl, the BSD
libcs, bionic, newlib, picolibc, relibc or llvm-libc), and no kernel's
source. glibc is used only as a black box: the reference
column of the differential (`tools/difftest`), its compiled behaviour
observed, never its source or its headers.

Every source consulted is recorded here, by the lane that consulted it.

## lc01 (2026-10-06): the first layer

**Standards and manuals**

| source | where | used for |
|---|---|---|
| ISO C17, draft N2176 | clause numbers as lc00's report cites them (7.5 errno, 7.22 stdlib, 7.24 string) | the citations in `src/` and `include/`; the text of each function's contract was checked against the POSIX pages below, which state they are aligned with ISO C |
| POSIX.1-2024 (XSH), as the `3p` pages of man-pages 6.19 on the Linux test host | `memmove(3p)`, `strchr(3p)`, `strncmp(3p)`, `memcmp(3p)`, `calloc(3p)`, `realloc(3p)`, `exit(3p)`, `_Exit(3p)`, `write(3p)`, `errno(3p)` | overlap, unsigned-char comparison, `char` conversion in `strchr`, the terminating null, `status & 0377`, ENOMEM, `realloc` of size 0 (implementation-defined), errno unspecified after success, `errno` macro or external identifier |
| the Linux man-pages project, 6.19 | `syscall(2)` (the x86-64 register table), `brk(2)` (the raw call answers the new break, or the current one on failure), `malloc(3)` (malloc(0), calloc overflow, realloc(p, 0)) | `sys.S`, `sys.lu`, `heap.lu` |
| System V AMD64 psABI | 3.2.3 parameter passing, 3.4.1 the initial process stack, A.2.1 the kernel convention | `sys.S`, `crt0.S` |
| Linux uapi headers, linux-api-headers 7.1 (the allowed paths; numbers and constants only) | `asm/unistd_64.h` (write 1, mmap 9, munmap 11, brk 12, exit_group 231), `asm-generic/errno-base.h` (the 34 E* numbers), `asm-generic/mman-common.h` and `linux/mman.h` (PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANONYMOUS) | `include/errno.h`, the constants in `src/` |

Package ownership was checked with `pacman -Qo` before each uapi file
was read, so no glibc header under `/usr/include` was opened.

**wolf and the project's records**

| source | used for |
|---|---|
| wolf-lang `spec/04-abi.md` and `spec/02-memory-model.md` at `v0.2.24` (`294d626d`): `[abi.c.export]`, `[abi.c.import]`, `[abi.c.types]`, `[abi.target]`, `[abi.target.none.hooks]`, `[abi.asm.link]`, `[abi.asm.roster]`, `[abi.link.extern]`, `[mem.unsafe.*]`, `[mem.static]`, `[mem.prov.expose]` | how muzzle exports, calls assembly, names static storage and handles pointers |
| wolf-lang driver test fixtures at `v0.2.24`: `crates/wolf_driver/tests/fixtures/freestanding_interrupt/wolf.pkg`, `freestanding_alloc/hooks.lu`, `freestanding_static/kmain_static.lu`, `c_membrane/export.lu`, `corpus/membrane/export_child.lu`, `geo/geo.lu` | the spelling of `asm:`, `extern "c" let`, raw-pointer methods, a child module's export |
| the libc track's plan: its index, lc00's design note and report, the track's rulings (the clean room, the kernel boundary, the library's name, the C membrane's shape, the four design rulings), the freestanding campaign's plan | the layering, the test method, the rulings |
| wolffe-lang/pax `docs/CENSUS.md` (px04) | the syscall set against the census |
| wolffe-lang/boreutils `LICENSE`, `LICENSE-TRAINING-DATA`, `README.md`, `wolf-toolchain.toml`, `.github/workflows/ci.yml` | the licences (copied), the pin and CI shapes |

**Not consulted:** any libc's documentation beyond the man-pages
project and POSIX, and any allocator's design or code. The allocator
(power-of-two classes, a header word, per-class free lists, brk then
mmap) is muzzle's own.

## lc02 (2026-11-01): streams and the printf family

**Standards and manuals**

| source | where | used for |
|---|---|---|
| ISO C17, draft N2176 | 7.21 `<stdio.h>`: 7.21.1 (the macros, `stdin`/`stdout`/`stderr` as expressions of type `FILE *`), 7.21.2 (streams), 7.21.3 (files; the three buffering modes and when a buffer is transmitted, p3; stdin and stdout fully buffered only when not interactive and stderr never fully buffered, p7), 7.21.5.1–7.21.5.6, 7.21.6.1 (the conversion grammar), 7.21.6.3–7.21.6.13, 7.21.7.1–7.21.7.10, 7.21.8.1–7.21.8.2, 7.21.10; 7.16 `<stdarg.h>`; 7.19; 7.20 `<stdint.h>`; 5.2.4.2.1 `<limits.h>`; 7.29 (the `wint_t` type); 7.22.4.4–7.22.4.5 (exit flushes, `_Exit` need not); 7.5 (`EILSEQ`) | `include/stdio.h`, `stdarg.h`, `stdint.h`, `limits.h`, `wchar.h`, and every function in `src/stdio/` |
| POSIX.1-2024 (XSH, XBD) | XSH fopen (the mode strings and their open flags), fdopen, fclose, fflush (an input stream's offset), setvbuf, fileno, fprintf (`EOVERFLOW`, `EILSEQ`, the `'` flag, `C`, `S`, the `%n$` forms muzzle refuses), fgetc, fputc, fputs, puts, ungetc, fread, fwrite, open, close, read, lseek, isatty, _exit; XBD stdio.h, fcntl.h, unistd.h, sys/types.h, limits.h | the same, `src/unistd/`, `include/fcntl.h`, `include/unistd.h`, `include/sys/types.h` |
| the Linux man-pages project | open(2), read(2), lseek(2), fstat(2) (`st_blksize`), fcntl(2) (`F_GETFL`), ioctl_tty(2) (`TCGETS`), stdio(3), setbuf(3), printf(3), posix_openpt(3), termios(3), pty(7) | `src/unistd/`, `src/stdio/`, `tests/lib/ptyrun.c` |
| System V AMD64 psABI | 3.5.7 (the `va_list` record) | `include/stdarg.h` (the compilers' builtins, named only) |
| Linux uapi headers, linux-libc-dev in the build container (the allowed paths; numbers and constants only, each file's package checked with `dpkg -S` first) | `asm/unistd_64.h` (read 0, close 3, fstat 5, lseek 8, ioctl 16, fcntl 72, openat 257), `asm-generic/fcntl.h` (the `O_` flags, `F_GETFL`), `linux/fcntl.h` (`AT_FDCWD`), `asm-generic/ioctls.h` (`TCGETS`), `asm/stat.h` (`st_blksize`'s offset), `asm-generic/errno.h` (`EOVERFLOW`, `EILSEQ`) | `include/fcntl.h`, `include/errno.h`, the constants in `src/` (`notes/lc02-evidence/uapi.log`) |

**glibc as a black box.** Where the standards leave a choice open, glibc
2.39's answer is the reference: observed by running programs against it
(`notes/lc02-probes/g1.c` … `g4.c`, the differential), never by reading
its source or its headers. The observations are in `notes/lc02-stdio.md`
§2 and `docs/LEDGER.md`.

**wolf and the project's records**

| source | used for |
|---|---|
| wolf-lang `spec/04-abi.md` at `v0.2.36` (`5adf3d91`): `[abi.c.variadic]` and its sub-clauses, `[abi.layout.query]`, `[abi.asm.roster]`, `[abi.target.none.codegen]`; `spec/02-memory-model.md`: `[mem.unsafe.raw.4]`, `[mem.unsafe.raw.5]`, `[mem.unsafe.sig]`, `[mem.static]`; `spec/01-grammar.md` §2 and §3.2 | the variadic entry points and `VaList`, raw field access through `*File`, the module layout (a directory is a module), operator precedence |
| wolf-lang `crates/wolf_driver/tests/fixtures/variadic/exports.lu` and `caller.c`, `corpus/comptime/layout_query_repr_c.lu` at `v0.2.36` | the spelling of `va_start`, `va_arg[T]`, `va_copy(ap[0])` for a `*VaList`, `va_end` |
| muzzle branch `s263` (`d110e867`, org-owned, unmerged) and wolf-lang `notes/s263-evidence/lc02-draft.txt` | a first printf over `VaList` (bare `%d %s %c %x %%`), read as a design reference; lc02's engine is written anew |
| lc02's contract, lc00's design note and report, lc01's contract and note | the scope, the layering, the test method |

**Not consulted:** any C library's source or headers (glibc, musl, the BSD
libcs); musl's and the BSD libcs' documentation, which the contract allows
as a design reference, was not needed. The stream design (a record with
one buffer that is either read-ahead or pending output, an open list,
flush points chosen to match glibc's observed ones) and the formatter
(one pass, digits most significant first without a buffer, two sinks) are
muzzle's own. The compilers' `__builtin_va_*` names in `<stdarg.h>` are
named, not read from any header.

## lc03 (2026-11-09): stdlib, errno and strerror

**Standards and manuals**

| source | where | used for |
|---|---|---|
| ISO C17, draft N2176 | 7.22.1 (the numeric conversions: the subject sequence, the bases, `ERANGE`, strtod's rounding and its INF/NAN forms), 7.22.2 (rand, srand), 7.22.3.1 (aligned_alloc), 7.22.4 (abort, atexit, exit, _Exit, getenv), 7.22.5 (bsearch, qsort), 7.22.6 (abs, div), 7.24.6.2 (strerror), 7.21.10.4 (perror), 7.5 (errno), 6.5.5 (division truncates toward zero), Annex F.5 (conversions round as the current rounding mode does) | `src/stdlib/`, `src/process/`, `src/errtext/`, `src/stdio/perror.lu`, `include/stdlib.h`, `include/errno.h` |
| POSIX.1-2024 (XSH, XBD) | XSH posix_memalign, setenv, unsetenv, getenv, abort, atexit, perror, strerror; XBD 8.1 (environ); XBD errno.h (`ENOTSUP`); XSH 2.3 (errno after a successful call) | the same, and `src/sys.S`'s `environ` |
| IEEE 754-2019 | binary64's fields, round to nearest with ties to even, subnormals | `src/stdlib/float.lu` |
| the Linux man-pages project | strtol(3), strtoul(3), strtod(3), qsort(3), bsearch(3), rand(3), setenv(3), getenv(3), environ(7), abort(3), atexit(3), strerror(3), perror(3), errno(3), posix_memalign(3), rt_sigaction(2), sigprocmask(2), tgkill(2), getpid(2), gettid(2) | the same |
| System V AMD64 psABI | 3.2.3 (a double result in `%xmm0`; a struct of two integers returned in `%rax`/`%rdx`; the argument registers a trampoline shifts) | `src/sys.S`'s `strtod`, `__muzzle_call2`, `__muzzle_call0`; `div_t` and `ldiv_t` by value |
| Linux uapi headers, linux-libc-dev in the build container (numbers and constants only; each file's package checked with `dpkg -S` first, `notes/lc03-evidence/red1c.log`) | `asm-generic/errno-base.h` and `asm-generic/errno.h` (every E* number), `asm/unistd_64.h` (rt_sigaction 13, rt_sigprocmask 14, getpid 39, gettid 186, tgkill 234), `asm/signal.h` (SIGABRT 6, x86-64's struct sigaction), `asm-generic/signal-defs.h` (SIG_UNBLOCK 1, SIG_DFL 0) | `include/errno.h`, `src/process/process.lu` |
| D. E. Knuth, The Art of Computer Programming, vol. 2, 3.3.4 | the MMIX linear congruential multiplier and increment | `src/stdlib/arith.lu` (rand) |

**glibc as a black box.** glibc 2.39's answers where the standards leave a
choice open (`notes/lc03-probes/g1.c`, `g2.c`, `g3.c`, and the cases'
reference column) are the reference, observed by running programs, never
by reading its source or headers. strerror's texts are glibc's own output:
`notes/lc03-probes/gen-errtext.py` writes `src/errtext.S` from what
`errno_strerror.c` printed against glibc, since the texts must equal glibc's.

**wolf**

| source | used for |
|---|---|
| wolf-lang `spec/04-abi.md` at `v0.2.36`: `[abi.c.export]` (a struct result by value, one symbol per export), `[abi.asm]`, `[abi.asm.link]` (several listed assembly files), `[abi.asm.roster]` | the trampolines, `strtod`'s entry, `errtext.S` |
| wolf-lang `spec/01-grammar.md` and `spec/10-types.md` at `v0.2.36`: the operator table, `INT` (hex literals with `_`), `[type.int.shift]`, `[type.wrap.div]`, `wrapping[T]` | the big-integer arithmetic and rand's wrapping product |
| `crates/wolf_rt/LICENSE-EXCEPTION` at wolf-lang `v0.2.36` | the shape of muzzle's exception, compared (no change) |

**Not consulted:** any C library's source or headers (glibc, musl, the
BSD libcs, or others); musl's and the BSD libcs' documentation, which is
allowed as a design reference, was not needed. qsort's stable bottom-up
merge sort, strtod's exact big-integer scaling with one rounding, the
aligned block inside a carrier, and the environment's copy-on-append are
textbook methods written here from the standards' contracts.

## lc03, the second part (2026-11-09): time and the file layer

**Standards and manuals**

| source | where | used for |
|---|---|---|
| ISO C17, draft N2176 | 7.27 `<time.h>` (7.27.1 struct tm's members, 7.27.2.1 clock, 7.27.2.3 mktime's normalization, 7.27.2.4 time, 7.27.3.3 gmtime, 7.27.3.4 localtime, 7.27.3.5 strftime and its C-locale table); 7.21.4.2 rename | `src/chrono/`, `include/time.h`, `src/unistd/fs.lu` |
| POSIX.1-2024 (XSH, XBD) | XSH clock_gettime, nanosleep, gmtime, localtime, mktime, timegm, strftime (the `0` and `+` flags, field widths, %s), open/openat, stat, fstat, lstat, unlink, rename, mkdir, rmdir, getcwd, chdir, dup, dup2, pipe, access, symlink, readlink, opendir, readdir, closedir; XBD 4.19 (seconds since the epoch), time.h (tm_gmtoff, tm_zone), sys/stat.h, dirent.h | the same, and the headers |
| ISO 8601 | the week-based year and week number (week 1 holds the year's first Thursday) | strftime's %G, %g and %V |
| H. Hinnant, "chrono-Compatible Low-Level Date Algorithms" (public domain) | `days_from_civil` and `civil_from_days` over 400-year eras | `src/chrono/civil.lu` |
| the Linux man-pages project | clock_gettime(2), nanosleep(2), stat(2), getdents64(2) (struct linux_dirent64), readdir(3) (d_type's names), access(2), getcwd(2), ctime(3), timegm(3), strftime(3) | the same |
| Linux uapi headers (numbers and constants only; each file's package checked with `dpkg -S` first, `notes/lc03-evidence/red1c.log`) | `asm/unistd_64.h` (stat 4, lstat 6, access 21, pipe 22, dup 32, dup2 33, nanosleep 35, getcwd 79, chdir 80, rename 82, mkdir 83, rmdir 84, unlink 87, symlink 88, readlink 89, getdents64 217, clock_gettime 228), `linux/time.h` (the clock ids), `linux/fcntl.h` (the AT_ flags), `linux/stat.h` (the mode bits, from which d_type's values follow), `asm/stat.h` (x86-64's struct stat) | `include/time.h`, `include/sys/stat.h`, `include/dirent.h`, `include/fcntl.h`, `src/chrono/`, `src/unistd/` |

**glibc as a black box.** glibc 2.39's answers where the standards leave
a choice open (the cases' reference column, `notes/lc03-probes/g1.c`) are
the reference: the zone names, mktime's tm_isdst rule, strftime's widths,
its negative years and what it copies back, never its source or headers.
DT_REG, DT_DIR and DT_LNK's values were also printed by `file_dir.c`
against glibc, and agree with S_IFMT's bits shifted down 12.

**Not consulted:** any C library's source or headers; musl's and the BSD
libcs' documentation was not needed. The DIR stream (a descriptor and a
buffer of the kernel's records, answered in place) and strftime's single
pass with a counting dry run for padded composites are muzzle's own.
