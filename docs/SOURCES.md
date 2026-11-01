# Sources

muzzle is written clean-room (the libc track's rule, STATUS ruling
#25): **no C library's source is read** (not glibc, musl, the BSD
libcs, bionic, newlib, picolibc, relibc or llvm-libc), and no kernel's
source (ruling #22). glibc is used only as a black box: the reference
column of the differential (`tools/difftest`), its compiled behaviour
observed, never its source or its headers.

Every source consulted is recorded here, by the lane that consulted it.

## lc01 (2026-10-06): the first layer

**Standards and manuals**

| source | where | used for |
|---|---|---|
| ISO C17, draft N2176 | clause numbers as lc00's report cites them (7.5 errno, 7.22 stdlib, 7.24 string) | the citations in `src/` and `include/`; the text of each function's contract was checked against the POSIX pages below, which state they are aligned with ISO C |
| POSIX.1-2024 (XSH), as the `3p` pages of man-pages 6.19 on kasumi | `memmove(3p)`, `strchr(3p)`, `strncmp(3p)`, `memcmp(3p)`, `calloc(3p)`, `realloc(3p)`, `exit(3p)`, `_Exit(3p)`, `write(3p)`, `errno(3p)` | overlap, unsigned-char comparison, `char` conversion in `strchr`, the terminating null, `status & 0377`, ENOMEM, `realloc` of size 0 (implementation-defined), errno unspecified after success, `errno` macro or external identifier |
| the Linux man-pages project, 6.19 | `syscall(2)` (the x86-64 register table), `brk(2)` (the raw call answers the new break, or the current one on failure), `malloc(3)` (malloc(0), calloc overflow, realloc(p, 0)) | `sys.S`, `sys.lu`, `heap.lu` |
| System V AMD64 psABI | 3.2.3 parameter passing, 3.4.1 the initial process stack, A.2.1 the kernel convention | `sys.S`, `crt0.S` |
| Linux uapi headers, linux-api-headers 7.1 (allowed paths, STATUS #22; numbers and constants only) | `asm/unistd_64.h` (write 1, mmap 9, munmap 11, brk 12, exit_group 231), `asm-generic/errno-base.h` (the 34 E* numbers), `asm-generic/mman-common.h` and `linux/mman.h` (PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANONYMOUS) | `include/errno.h`, the constants in `src/` |

Package ownership was checked with `pacman -Qo` before each uapi file
was read, so no glibc header under `/usr/include` was opened.

**wolf and the planning record**

| source | used for |
|---|---|
| wolf-lang `spec/04-abi.md` and `spec/02-memory-model.md` at `v0.2.24` (`294d626d`): `[abi.c.export]`, `[abi.c.import]`, `[abi.c.types]`, `[abi.target]`, `[abi.target.none.hooks]`, `[abi.asm.link]`, `[abi.asm.roster]`, `[abi.link.extern]`, `[mem.unsafe.*]`, `[mem.static]`, `[mem.prov.expose]` | how muzzle exports, calls assembly, names static storage and handles pointers |
| wolf-lang driver test fixtures at `v0.2.24`: `crates/wolf_driver/tests/fixtures/freestanding_interrupt/wolf.pkg`, `freestanding_alloc/hooks.lu`, `freestanding_static/kmain_static.lu`, `c_membrane/export.lu`, `corpus/membrane/export_child.lu`, `geo/geo.lu` | the spelling of `asm:`, `extern "c" let`, raw-pointer methods, a child module's export |
| wolffe-lang/wolf: `sprints/libc/index.md`, lc00's design note and report, STATUS rulings #22 #25 #26 #31 #32, the KWC campaign file | the layering, the test method, the rulings |
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
| Linux uapi headers, linux-libc-dev in the build container (allowed paths, STATUS #22; numbers and constants only, each file's package checked with `dpkg -S` first) | `asm/unistd_64.h` (read 0, close 3, fstat 5, lseek 8, ioctl 16, fcntl 72, openat 257), `asm-generic/fcntl.h` (the `O_` flags, `F_GETFL`), `linux/fcntl.h` (`AT_FDCWD`), `asm-generic/ioctls.h` (`TCGETS`), `asm/stat.h` (`st_blksize`'s offset), `asm-generic/errno.h` (`EOVERFLOW`, `EILSEQ`) | `include/fcntl.h`, `include/errno.h`, the constants in `src/` (`notes/lc02-evidence/uapi.log`) |

**glibc as a black box.** Where the standards leave a choice open, glibc
2.39's answer is the reference: observed by running programs against it
(`notes/lc02-probes/g1.c` … `g4.c`, the differential), never by reading
its source or its headers. The observations are in `notes/lc02-stdio.md`
§2 and `docs/LEDGER.md`.

**wolf and the planning record**

| source | used for |
|---|---|
| wolf-lang `spec/04-abi.md` at `v0.2.36` (`5adf3d91`): `[abi.c.variadic]` and its sub-clauses, `[abi.layout.query]`, `[abi.asm.roster]`, `[abi.target.none.codegen]`; `spec/02-memory-model.md`: `[mem.unsafe.raw.4]`, `[mem.unsafe.raw.5]`, `[mem.unsafe.sig]`, `[mem.static]`; `spec/01-grammar.md` §2 and §3.2 | the variadic entry points and `VaList`, raw field access through `*File`, the module layout (a directory is a module), operator precedence |
| wolf-lang `crates/wolf_driver/tests/fixtures/variadic/exports.lu` and `caller.c`, `corpus/comptime/layout_query_repr_c.lu` at `v0.2.36` | the spelling of `va_start`, `va_arg[T]`, `va_copy(ap[0])` for a `*VaList`, `va_end` |
| muzzle branch `s263` (`d110e867`, org-owned, unmerged) and wolf-lang `notes/s263-evidence/lc02-draft.txt` | a first printf over `VaList` (bare `%d %s %c %x %%`), read as a design reference; lc02's engine is written anew |
| wolffe-lang/wolf `sprints/libc/02-stdio/lc02-stdio.md`, lc00's design note and report, lc01's contract and note | the scope, the layering, the test method |

**Not consulted:** any C library's source or headers (glibc, musl, the BSD
libcs); musl's and the BSD libcs' documentation, which the contract allows
as a design reference, was not needed. The stream design (a record with
one buffer that is either read-ahead or pending output, an open list,
flush points chosen to match glibc's observed ones) and the formatter
(one pass, digits most significant first without a buffer, two sinks) are
muzzle's own. The compilers' `__builtin_va_*` names in `<stdarg.h>` are
named, not read from any header.
