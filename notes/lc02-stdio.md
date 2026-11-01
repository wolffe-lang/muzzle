# lc02: stdio and the printf family

Lane lc02 (crawl 54, batch A, the libc track, Opus). Contract: wolffe-lang/wolf
`sprints/libc/02-stdio/lc02-stdio.md` at planning trunk `97331d5`, under
`sprints/the-lane-rules-1031.md` and `sprints/RULES.md` §1–§10. Base: muzzle
trunk `8d5896fa` (wolf 0.2.36). Branch `lc02`.

This file holds §2 as re-derived and §3 as committed before the first line of
library code. §1, §4 and §5 are filled in at the end.

## 2. Inputs, verified (re-derived 2026-11-01)

| input as written | at origin | drift |
|---|---|---|
| muzzle trunk `8d5896fa`, wolf 0.2.36 | `origin/trunk` = `8d5896faf7d1…`; `wolf-toolchain.toml` pins 0.2.36, rev `5adf3d91`, version line `wolf 0.2.36 (wolfgang, pin 5adf3d9)`, linux x86-64 `d383c74c8cd1…` | none |
| the pinned archive | tag `v0.2.36` → `5adf3d91f75b…`; release 427033151 (published 2026-10-28T19:20:00Z), asset digest `sha256:d383c74c8cd1…`, 82,827,649 bytes (read 1, the release API); `sha256sum` twice on hasu of the copy taken from r41p's read-back (read 2) `d383c74c8cd1…`; `tools/fetch-toolchain` checks it a third time in every run | none |
| the build host: hasu's container `localhost/px75-ubuntu:2`, a block per run | clang 18.1.3, glibc 2.39, kernel 7.2.4, x86-64; `script`, `nm`, `readelf`, `objdump`, `python3`, `od`, `cmp`, `timeout` present; `strace` and `xxd` absent; `/dev/full` and `/dev/ptmx` present. The same clang and glibc as CI's ubuntu-latest (lc01's runs: glibc 2.39, clang 18.1.3) | none |
| the baseline at trunk | block B, job `explore1`: `tests/stale-compiler` 5/5, `tools/build`, `tools/difftest` 9/9 on both tiers, every step exit 0 (`notes/lc02-evidence/explore1.log`) | none |
| #515 C variadics in 0.2.36 | closed 2026-10-25; `[abi.c.variadic]` at `v0.2.36`: `VaList` (24 bytes), `va_start`/`va_arg[T]`/`va_copy`/`va_end` in `unsafe`, `*VaList` for C's decayed `va_list`; supported on `x86_64-unknown-none`, where "wolf floating values remain refused" | see the probes: `VaList`'s fields cannot be read from wolf |
| what lc01 said lc02 waits on | #598 closed (2026-10-07); #516 (exported data), #597 (a var's address), #599 (C-only modules), #604 (a literal's bytes), #520 (C function pointers), #517 (TLS), #522 (loop idiom) open | none. So `stdin`, `stdout`, `stderr` and the standard streams' storage live in `sys.S`, named with `extern "c" let`, as errno does; literal text (`(null)`, `(nil)`) too |
| s263's acceptance branch (muzzle `s263`, `d110e867`) | unmerged; a bare `%d %s %c %x %%` printf over `VaList`, read as org-owned design reference. Its draft contract for lc02 (`wolf-lang notes/s263-evidence/lc02-draft.txt`) is a different, smaller scope than this lane's contract; the contract governs | none |

### The probes (`notes/lc02-probes/`, jobs `explore1`–`explore3`)

Each wolf probe is built for `x86_64-unknown-none` on both tiers with
`--deny-warnings`, linked with trunk's `crt0.o` and `libmuzzle.a`, and run.

| probe | question | answer, both tiers |
|---|---|---|
| q1 | one module across two files (directory = module) | yes: private fns, a private `#[repr(c)]` struct and a `*u8` fn in `b.lu` used from `a.lu`; a `pub` item needs a `///` (W0313 under `--deny-warnings`) |
| q2 | a `#[repr(c)]` struct with a pointer field, read and written through `*F` (`f[0].x`, `(*f).x`, `f[0].x \|= 4`) | yes, at clang's offsets (24 bytes; 8, 16). A raw index is `int` (E0401 for `i64`) |
| q2b | a struct local passed by `mut`, written by the callee | yes |
| q3a | `VaList`'s fields (`ap.gp_offset`) | **no**: "cannot compile this yet — members on this type (the std surface)" |
| q3b | `va_arg[f64]` | **no**: "a floating-point value (`f64`) on target x86_64-unknown-none (K10)" |
| q8 | a `VaList` kept behind `*VaList`, its fields read raw at the documented offsets, a double's bits taken by hand | see `explore3` |
| q4, q7 | integer operations | `<<` is overflow-checked (it trapped on high bits); `as` between integer types is value-checked (a negative `i64` `as u64` trapped). The wrapping forms are `as wrapping[T]` on both operands |
| q5 | module `var`s (an address as `u64`, a `bool`) across calls | yes (#598 is fixed) |

### glibc as the oracle (`g1`–`g4`, black box only)

glibc 2.39's choices where the standards leave them open, as the differential
will see them. Among them: `%p` of a null pointer is `(nil)` and honours `+`,
space, `0` and precision otherwise; `%s` of a null pointer is `(null)` unless a
precision below 6 makes it empty; `%5%` ignores the width; an unknown
conversion prints `%` and its character, dropping the length; a call that
fails (`EINVAL` for an incomplete specification, `EILSEQ` for `%lc` of a
non-ASCII value in the C locale, `EOVERFLOW` past `INT_MAX`) writes none of
its own output. A full buffer is written when the next byte arrives; a write
that does not fit drains the buffer and itself; the default buffer of a
regular file is 4096 bytes; a failed flush discards the buffer, so `fclose`
afterwards returns 0; `fputs` returns 1 and `puts` the bytes it wrote;
`ungetc` pushes back more than one byte, last in first out; `"rw"` opens as
`"r"`.

## 3. Prediction, committed before the first change

**The export list: 35 C functions and 3 data objects**, every function written
in wolf, exported with its C name on both compiling tiers:

| header | functions |
|---|---|
| `<stdio.h>` streams (22) | `fopen` `fdopen` `fclose` `fflush` `fread` `fwrite` `fputc` `putc` `putchar` `fputs` `puts` `fgetc` `getc` `getchar` `fgets` `ungetc` `feof` `ferror` `clearerr` `fileno` `setvbuf` `setbuf` |
| `<stdio.h>` printf (8) | `printf` `fprintf` `sprintf` `snprintf` `vprintf` `vfprintf` `vsprintf` `vsnprintf` |
| `<unistd.h>`, `<fcntl.h>` (5) | `open` `close` `read` `lseek` `isatty` |
| data, in `sys.S` | `stdin` `stdout` `stderr` (`FILE *`, each pointing at a slot `sys.S` reserves) |

The headers are hand-written: `<stdio.h>`, `<stdarg.h>`, `<fcntl.h>`,
`<stdint.h>`, `<limits.h>`, `<wchar.h>` (types only), `<sys/types.h>`, and
`<unistd.h>` grows. lc01's 19 exports stay.

**The syscalls: 7 new, 12 in all.** `read` 0, `close` 3, `fstat` 5 (a stream's
buffer is its file's `st_blksize`, as glibc's 4096 shows), `lseek` 8, `ioctl` 16
(`isatty` as `TCGETS`), `fcntl` 72 (`fdopen` checks the descriptor's access
mode), `openat` 257. Falsified by any other, or if `fstat` or `fcntl` is not
needed.

**R-lc02-1 predicted B**: the floating conversions go to lc03 and this lane
refuses them by name. The probes already show why (A) costs more than this
lane can carry: `f64` is refused on muzzle's only target, `VaList`'s fields
cannot be read, so a double reaches wolf only by hand-walking a `VaList` record
kept behind a raw pointer (q8), and the decimal conversion has to be exact in
integer arithmetic (a bignum of up to 1,074 fractional bits), then held to
100,000 doubles per conversion.

| # | predicted | falsified if |
|---|---|---|
| P1 | all 35 functions are wolf; the new non-wolf code is `sys.S` storage only (the three standard `FILE` slots, `stdin`/`stdout`/`stderr`, a termios scratch for `isatty`, the literal texts) | a function needs C or new assembly code |
| P2 | 7 new syscalls, as listed | another is needed, or `fstat` or `fcntl` is not |
| P3 | **13 new case files**, one per family: `unistd`, `stdio_open`, `stdio_char`, `stdio_rw`, `stdio_ungetc`, `stdio_buf`, `stdio_tty`, `stdio_err`, `stdio_exit`, `stdio_exit_` (`_exit` does not flush), `printf`, `printf_grammar` (the generated corpus), `snprintf`, plus `vprintf`: 14 at most. 22–23 cases agree with glibc on both tiers at the head: stdout, stderr, exit status and every file the case wrote | any case disagrees on either tier, or a tier needs a case the other does not |
| P4 | the harness grows four comparisons (each case in its own directory, its written files compared; stderr compared; stdin fed from `<case>.in`; a `.tty` case run with stdout on a pseudo-terminal) and each is seen red by a self-test before it is trusted | a comparison is never seen red |
| P5 | the printf corpus from the grammar (flags × width × precision × length × value edges, for `d i u o x X`, plus `c s p %` and `n`) is over 300,000 calls and agrees byte for byte | it disagrees anywhere, or is smaller |
| P6 | **8 to 12 new ledger rows**, every one a place where C17 or POSIX leaves the choice open and muzzle matches glibc, plus one row for the floating conversions' refusal (R-lc02-1) and one for positional arguments (`%n$`, not provided) | fewer than 8, or a row where muzzle disagrees with glibc on a case |
| P7 | the plant (`%X` printing lower case) is red by name on both tiers in exactly the cases whose output has a `%X` conversion (`printf`, `printf_grammar`, `snprintf`), green elsewhere | another case goes red, or one of those stays green |
| P8 | the archive gate stays at 0 imports on both tiers (no `memcpy`/`memset` from wolf-lang#522 outside muzzle's own) | `nm -u` shows an import |
| P9 | wolf limitations met that a lane would file: 1 to 3 (one known: `VaList`'s fields) | none, or more than 3 |
| P10 | R-lc02-1 = B, recommended with the evidence above; the refusal is a `-1` return with errno `EINVAL` and none of the call's output, tested by a muzzle-only self-checking case | (A) turns out cheap enough to do here |
