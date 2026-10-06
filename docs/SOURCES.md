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
| POSIX.1-2024 (XSH), as the `3p` pages of man-pages 6.19 on kasumi | `memmove(3p)`, `strchr(3p)`, `strncmp(3p)`, `memcmp(3p)`, `calloc(3p)`, `realloc(3p)`, `exit(3p)`, `_Exit(3p)`, `write(3p)`, `errno(3p)`, `malloc(3p)` | overlap, unsigned-char comparison, `char` conversion in `strchr`, the terminating null, `status & 0377`, ENOMEM, `realloc` of size 0 (implementation-defined), errno unspecified after success, `errno` macro or external identifier |
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
