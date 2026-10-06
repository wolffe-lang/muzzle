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
