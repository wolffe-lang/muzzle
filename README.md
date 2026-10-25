# muzzle

The wolf C library: a C17/POSIX libc for x86-64 Linux, written in
[wolf](https://github.com/wolffe-lang/wolf-lang). Named for the part of a
wolf that meets the world, and for the libc it is not (STATUS #26).
muzzle talks to the kernel only through the Linux x86-64 syscall ABI,
so a program linked against it statically is, unchanged, a userspace
test for PAX.

**State: the first layer (lc01).** Raw syscalls, errno, the memory and
string core, the allocator and process exit, enough for a C program
built with `clang -nostdlib` to run against muzzle alone. This acceptance
branch adds the first `printf` slice (`%d %s %c %x %%`) using the
compiler's new C variadic support. It requires an unreleased compiler build;
the release pin is
intentionally unchanged, so this branch is not ready to merge as-is.

## What C gets

| header | functions |
|---|---|
| `<string.h>` | `memcpy` `memmove` `memset` `memcmp` `strlen` `strcmp` `strncmp` `strchr` `strcpy` |
| `<stdlib.h>` | `malloc` `free` `calloc` `realloc` `exit` `_Exit` |
| `<stdio.h>` | `printf` (bare `%d %s %c %x %%` only) |
| `<unistd.h>` | `_exit` `write` |
| `<errno.h>` | `errno` (`(*__errno_location())`), the 34 base E* numbers |
| `<stddef.h>` | `size_t` `ptrdiff_t` `wchar_t` `max_align_t` `NULL` `offsetof` |

```sh
clang -std=c17 -fno-builtin -fno-stack-protector \
      -nostdinc -isystem muzzle/include -nostdlib -static \
      build/native/crt0.o prog.c build/native/libmuzzle.a -o prog
```

`crt0.o` holds `_start`, which calls `main(argc, argv, envp)` and then
`exit`. `-fno-stack-protector` is needed until muzzle sets up TLS
(`docs/LEDGER.md`).

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
| `src/stdio/` | the first, unbuffered `printf` slice |
| `src/process/` | `exit`, `_Exit`, `_exit`, `write` |
| `src/sys.S` | the one syscall routine and the static storage wolf cannot hold yet (ruling R2) |
| `src/crt0.S` | `_start` |
| `include/` | the C headers, hand-written from the standards' synopses (ruling R3: muzzle's one permanent C) |
| `tests/cases/` | the differential's cases, one per function family |
| `tests/stale-compiler` | `tools/build` runs the pinned compiler or refuses: a stale one is staged on purpose (muzzle#4) |
| `tools/` | `fetch-toolchain`, `build`, `difftest` |
| `docs/SOURCES.md` | every source consulted (the clean-room record) |
| `docs/LEDGER.md` | open behaviours the cases observe, and the link's departures |
| `notes/` | lane notes, probes and evidence logs |

## Build and test

On an x86-64 Linux host with clang:

```sh
tools/fetch-toolchain        # wolf 0.2.34 by digest (wolf-toolchain.toml)
tools/build native           # or release; gates the archive's symbols
tools/difftest native        # every case against glibc; non-zero on any difference
```

`tools/difftest` compiles each case twice: against the host glibc, a
black box, and statically against muzzle alone. It runs both and
compares stdout and the exit status. A case that fails to build in
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

### Development acceptance

Before a variadic release is pinned, run in an x86-64 Linux
container with `WOLF` naming the source-built driver,
`WOLF_EXPECT_VERSION` its exact first `--version` line, and
`S263_COMPILER_HEAD` its source head/stamp, then `tools/printf-acceptance`.
The override is loud, explicit, and does not relax the default pin check.
The runner retains each attempt under `notes/s263/run-<UTC timestamp>/`,
including commands, true exits, driver/corpus/output SHA-256 digests,
fifty primary calls and their independent byte counts, the legacy suite,
muzzle-only invalid-format/closed-output checks, and an isolated count-only
defect that the differential must reject on both tiers.
Only `x86_64-unknown-none` is built. This branch cannot be merged until a
released compiler is pinned and the normal release-based CI is green.
