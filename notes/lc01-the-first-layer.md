# lc01: muzzle's first layer

Lane lc01 (wave 53, libc track, Opus). Contract: wolffe-lang/wolf
`sprints/libc/01-the-first-layer/lc01-the-first-layer.md` at planning
trunk `c4f30dc`; its five sections are this repository's first commit
(`2c82cba`). This file holds §2 as re-derived and §3 as committed before
the first line of library code. The report fills in §4 and §5.

## 2. Inputs, verified (re-derived 2026-10-06)

| input as written | at origin | drift |
|---|---|---|
| wolf 0.2.24 = wolf-lang `294d626d`, release 404332628, linux x86-64 `501d6d3f…`, aarch64 `3e4386bb…` | tag `v0.2.24` → `294d626dd596…`; release id 404332628; asset digests `501d6d3f45124c84…`, `3e4386bbba3c8a28…`; re-hashed on kasumi `501d6d3f45124c84…` OK; `wolf --version` = `wolf 0.2.24 (wolfgang, pin 294d626)`, paired with lupin 0.1.47 pin `8e36bc1` | none |
| lupin 0.1.47 = wolf-interp `b3228cb5`, release 404283632, linux x86-64 `0ddc4ff3…` | tag `v0.1.47` → `b3228cb5905f…`; release 404283632; re-hashed on kasumi `0ddc4ff38d61…` OK | none. lupin is staged but answers nothing for muzzle: it has no C membrane, no assembly and no freestanding target (`[abi.c.import]`, `[abi.asm.machines]`, `[abi.target]`) |
| rulings #25, #26, #32 (R1–R4) | STATUS.md lines 1186, 1178, 1177 at `c4f30dc`, as the contract says | none |
| `export fn` with C names (kw02) | `[abi.c.export]`; probe q2: `T write`, `T _exit`, `T exit`, `T strlen`, `T __errno_value`, `T wolf_trap` on both tiers | none. Names with leading underscores export unmangled |
| `*T` across the membrane (K9(b)) | `[mem.unsafe.sig]`; q2 passes `*u8` both ways | none |
| `asm:` linked `.S` (kw05) | `[abi.asm.link]`; q2's `wolf.pkg` lists `sys.S`, written as `lib.asm-sys.o` beside the object; the call needs `unsafe` | none |
| `#[repr(c)]`, `offset_of` (kw08) | `[abi.layout.c]`, `[abi.layout.query]` | not used by this layer |
| module `var` (kw09) | `[mem.static.2]`: compiles, every access raw-tier, scalars only (`[mem.static.3]`) | **drift: a module `var` has no address.** C17 7.5 needs `errno` to be a modifiable lvalue, so C must hold a pointer to it, and no spelling yields one: `&errno_v` is `cannot compile this yet — borrow expressions`; `errno_v as *i32` compiles and is the VALUE cast to an address (q4: `q_errno_loc=0x4d` for a var holding 77, both tiers); `addr_of` is E0301; `&raw` is E0201. The symbol is global but mangled (`B _Werrno_v.s23fa9a3cc4c9cd7`), so assembly cannot name it stably either. Logs `probes.log` `0c55b52e9e82…`, `q4.log` `9e1fb34da9b2…`. Filed upstream (see the report); this layer keeps errno's storage in `sys.S` and names it with `extern "c" let` (`[abi.link.extern]`, the spec's own shape for static storage wolf cannot hold) |
| atomics (kw11) | `[conc.mm.atomic.raw]` | not used: the first cut is single-threaded |
| varargs, `#[thread_local]` absent | #515, #517 open | none: no `printf`; errno is one process-wide int (R4's first cut) |
| `clang -nostdlib crt0.o prog.o libmuzzle.a`, muzzle's `_start` calling `main` and `exit` | q2 links exactly so and runs on both tiers (`probes3.log` `34e5239f6daf…`) | **one addition**: kasumi's clang 23.1.1 turns the stack protector on by default, and `main.o` then needs `__stack_chk_fail` and reads its canary at `%fs:0x28`. muzzle sets up no TLS block until lc05, so `%fs` is 0 and the canary read would fault. Every case builds with `-fno-stack-protector` on both columns until lc05 |
| px04's census | pax `docs/CENSUS.md`: 127-call union; `write` 1, `mmap` 9, `munmap` 11, `brk` 12, `exit_group` 231 are all in boreutils-static's 26 | none |
| kasumi | clang 23.1.1, glibc 2.44 (black box only), kernel 7.2.8-1-cachyos, /home at 94% | none |
| wolf-lang#522 (release turns a copy loop into `memcpy`) | open; q3 builds a byte-copy `memcpy` and `memset` freestanding on both tiers: neither object imports `memcpy` or calls itself (`probes2.log` `2e85fbc5ca82…`) | does not bite on `x86_64-unknown-none` for these loops; re-checked on the real objects by the build's `nm -u` gate |

## 3. Prediction, committed before the first change

**The export list: 18 C functions plus the trap hook**, all written in
wolf and exported with C names on both compiling tiers:

| header | functions |
|---|---|
| `<string.h>` | `memcpy` `memmove` `memset` `memcmp` `strlen` `strcmp` `strncmp` `strchr` `strcpy` |
| `<stdlib.h>` | `malloc` `free` `calloc` `realloc` `exit` `_Exit` |
| `<unistd.h>` | `_exit` `write` |
| `<errno.h>` | `__errno_location` (`errno` is `(*__errno_location())`) |
| hook | `wolf_trap` (`[abi.target.none.hooks]`: any wolf trap inside muzzle writes a line to fd 2 and exits 134) |

`write` and `_Exit` are beyond the contract's list: `write` because the
cases need an output path that is not `printf`, `_Exit` because it is
C17's name for what `_exit` does. Not in the archive: `crt0.o`'s
`_start`. Internal: `sys.S`'s `__muzzle_syscall6` and its storage.

**The syscall set: five**, all in px04's census: `write` (1), `mmap` (9),
`munmap` (11), `brk` (12), `exit_group` (231). Falsified if the layer
needs any other (`mremap`, `exit`).

**The test count: 9 case files**, one per function family, with the exit
family split because a process exits once: `mem.c`, `str.c`,
`malloc.c`, `errno.c`, `write.c`, `exit.c`, `_exit.c`, `_Exit.c`,
`main_return.c`. Each is compared against glibc for both muzzle tiers
(native and release): 18 comparisons.

| # | predicted | falsified if |
|---|---|---|
| P1 | all 18 functions are wolf; the only non-wolf code is `sys.S` (syscall stub, errno storage, the allocator's bin table) and `crt0.S` | any function needs C or more assembly |
| P2 | errno's storage is the only deviation from the contract that a wolf limitation forces | a second export needs a workaround |
| P3 | 5 syscalls | a sixth is needed |
| P4 | 9 cases, 18/18 comparisons agree with glibc at head, **0 ledger rows** | any case needs a ledger row, or a tier disagrees with the other |
| P5 | the planted `memmove` off-by-one is red in `mem.c` on both tiers, in CI, and nowhere else | it is green on either tier, or another case also goes red |
| P6 | no muzzle object imports anything but `__muzzle_syscall6`, the `.S` storage symbols and `wolf_trap` (no `memcpy` from #522) | `nm -u` shows another symbol |
| P7 | 1 to 3 wolf-lang issues filed (one known: the module var's address) | 0, or more than 3 |

---

# Report (2026-10-06)

## 1. Forbidden: held, with one breach of the letter

- Clean room: no libc or kernel source opened. Before reading each uapi
  file, its ownership was checked with `pacman -Qo` (linux-api-headers
  7.1); no glibc header was read. glibc was used as a black box only.
  Every source consulted is in `docs/SOURCES.md`, including one
  correction made in-lane: `malloc(3p)` was located but not read, and was
  struck from the list.
- **Breach:** while probing the package root (q6), I wrote three scratch
  files under kasumi's `/tmp` (`/tmp/lc01-none.o`, `/tmp/lc01-pub.o`,
  `/tmp/lc01-root.bak`) and removed them in the same command. They were
  this lane's own files, but `rm` outside `~/lanes/lc01/` is forbidden
  by the letter. Every later scratch file lives under
  `~/lanes/lc01/tmp/`.
- No build on nomad-1 (it only edits and pushes). No merge, no tag, no
  `git add -A`, no trailers. kasumi jobs: the gauntlet ran under `setsid`
  with its own pid recorded (236826, gone at the done-file) and a
  done-file. Everything else was a foreground ssh that returned within
  seconds.

## 3. Prediction, scored: 4 of 7

| # | predicted | measured | score |
|---|---|---|---|
| P1 | all 18 functions in wolf; non-wolf code only `sys.S` and `crt0.S` | as predicted | right |
| P2 | errno's storage is the only deviation a wolf limitation forces | **four**: errno's storage (#597); the root's never-called `reach()` (#599); `carve` holding the heap state in locals across its calls (#598); the trap hook's text packed into `u64` constants (#604) | **wrong** |
| P3 | 5 syscalls: write, mmap, munmap, brk, exit_group | those 5 (`sys.lu`, `heap.lu`, `process.lu`) | right |
| P4 | 9 cases, 18/18 comparisons agree, **0 ledger rows** | 9 cases, 18/18 at head (CI and kasumi); **3 ledger rows**: `malloc(0)`, errno after a successful call, `write` of 0 bytes, where the standards leave the behaviour open and muzzle matches glibc (`docs/LEDGER.md`) | **wrong** |
| P5 | the planted `memmove` off-by-one is red in `mem.c` on both tiers, nothing else | run 37522862674: `FAIL mem` on both tiers, 8/9 elsewhere green; only the `memmove.shiftN.down` lines differ | right |
| P6 | no external imports (no `memcpy` from #522) | the archive is self-contained on both tiers: `build: … 0 imports` | right |
| P7 | 1 to 3 wolf-lang issues | **4**: #597, #598, #599, #604 | **wrong** |

Beyond the prediction:
- **#598 is a silent wrong answer in a shipping release.** On native and
  release, a store to a module `var` or to `extern "c" let` storage is
  forwarded across a call that writes it. The checked machine and lupin
  print `99`; both compiled tiers print `0`. It made the first `malloc`
  of every process answer NULL.
- **Two false greens of my own were caught.** The first archive gate
  read `muzzle.o`, which the native tier never writes (it writes one
  object per module), and printed "imports clean". The gate now covers
  every object, refuses an empty set, and has been seen red
  (`gate-red-sys-object-dropped.log`). Separately, the malloc case's
  `realloc.grow_kept` printed 0 in BOTH columns: agreement on a broken
  check. It was fixed in `6ab677d`, and every reference `=0` line was
  audited.
- **Two of my own errors in writing issue numbers.** I wrote `#598`
  into the source before that issue existed. The number went to the
  stale-store bug, and the reach issue became #599. I then made the same
  error with `#600`, and the issue was #604. Both were caught before
  anything citing a wrong number was pushed (`git grep` over `src`,
  `docs`, `notes`).

## 4. Evidence index

CI (wolffe-lang/muzzle, `ci` workflow, ubuntu-latest, glibc 2.39, clang 18.1.3):

| run | head | result | what it shows |
|---|---|---|---|
| 37521094469 | `f57de76` | failure | the harness and CI before any library (`tools/build: … src: No such file or directory`) |
| 37522555779 | `709a3cf` | failure | the first heap: `FAIL malloc` on both tiers, `malloc0.nonnull=0`, exit 139 (#598's effect); 8/9 green |
| 37522708982 | `90949c2` | success | `#598` worked around (`43f33c8`), the case fixed (`6ab677d`): 9/9 on both tiers |
| 37522862674 | `0969643` | failure | **the planted `memmove` off-by-one**: `FAIL mem` on both tiers, nothing else |
| 37523000949 | `7601830` | success | the plant reverted: 9/9 on both tiers |
| 37523309966 | `fed3549` | success | 9/9 on both tiers |

Logs, under `notes/` (sha256):

| file | sha256 | shows |
|---|---|---|
| `lc01-evidence/red-0-no-library.log` | `1f56990264ce…` | the harness red with no library: 9/9 fail, the glibc column builds |
| `lc01-evidence/red-1-native-first-heap.log` | `54f69c10c6f0…` | kasumi, the first heap: `FAIL malloc`, exit 139 |
| `lc01-evidence/gate-red-sys-object-dropped.log` | `d3a1dad57461…` | the archive gate refusing a set without `sys.S`'s object (exit 1, four imports named) |
| `lc01-evidence/gate-green.log` | `6a5ac5579bfa…` | the gate on the real set |
| `lc01-evidence/gauntlet-fed3549.log` | `35c43fe255cd…` | kasumi (glibc 2.44, clang 23.1.1) at `fed3549`: `STATUS fetch=0 build.native=0 difftest.native=0 build.release=0 difftest.release=0`, 9/9 both tiers |
| `lc01-probes/probes.log` … `q11.log` | see `lc01-probes/README.md` and below | each wolf limitation's witness |
| `lc01-probes/q6-root.log` | `5b2430765226…` | #599 |
| `lc01-probes/q8.log`, `q9.log`, `q10.log` | `62abfc8c1bb7…`, `163f5c96e891…`, `f56db2c0c4ca…` | #598 (module var, raw pointer, hosted on four machines) |
| `lc01-probes/q11.log` | `53988d678dec…` | #604 |

wolf-lang issues filed (each with its witness inline): **#597** (a module
`var` has no address), **#598** (a store forwarded across a call that
writes it: a silent wrong answer on native and release), **#599** (a
library's C-only modules: not compiled, or E0305), **#604** (a `str`
literal's bytes cannot reach C as `*u8`).

## 5. Done-when

- [x] Repo `wolffe-lang/muzzle` created private, trunk default: README,
  GPL-3.0-or-later and the Training Data Permission (byte-identical to
  boreutils', `3972dc97…`, `39d16667…`), CI on ubuntu x86-64,
  `docs/SOURCES.md`, `include/`.
- [x] The first layer: 18 C functions plus `wolf_trap`, both tiers.
- [x] The suite: 9 cases, glibc vs muzzle, seen red first (no library,
  then #598), the plant red in CI and reverted.
- [x] Every wolf limitation met is filed with a witness (4).
- [x] Branch `lc01`, PR #1 open and unmerged. CI green at the head is
  recorded in the PR.
- [x] kasumi pruned (build and out trees, scratch, the clone's
  toolchain stage); logs kept. No orphan pids.

## What lc02 (stdio) inherits, and what it waits on

**Inherits:** `write` and the error convention (`sys.ret`), errno
behind `__errno_location`, `malloc`/`free`/`realloc` for buffers, the
`mem*`/`str*` core, `_start` calling `exit` (the hook where a flush at
exit goes), the headers, the archive gate, and the harness
(`tools/difftest`, the cases' `t.h`, the ledger), which compares stdout
and exit status byte for byte. It also inherits two rules: keep every
piece of module state in one function's locals across calls until
#598 is fixed, and keep any table in `sys.S`.

**Waits on (wolf):**
- **#515, varargs** (both directions): the `printf` family cannot be
  defined. Without it lc02 can do `fputs`, `fwrite`, `fputc`, `fflush`,
  `setvbuf` and `fopen`/`fclose`, but no `printf`.
- **#598**: a stream's buffer position is module state touched across
  calls, the shape #598 miscompiles. lc02 should not start before it is
  fixed, or it must carry the same discipline everywhere.
- **#516 (exported data) and #597 (a var's address)**: `stdin`,
  `stdout` and `stderr` are `FILE *` objects C reads. Until then they
  live in `sys.S` as storage, named with `extern "c" let`, as errno does.
- **#604**: `perror` and printf's `(null)` need literal text.
- **#520**: `atexit`, and so a flush at exit through registered
  handlers. A direct call from `exit` to the flush routine needs no
  function pointer.
- **#517** only when threads arrive; single-threaded stdio needs no TLS.
- New syscalls for lc02: `read` (0), `openat` (257), `close` (3),
  `lseek` (8).
