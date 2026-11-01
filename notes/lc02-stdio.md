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
| q8 | a `VaList` kept behind `*VaList`, its fields read raw at the documented offsets, a double's bits taken by hand | yes, on both tiers (`explore3`: `q6.ok=1`): ten doubles and ten longs, through both the register save area and the overflow area. (q6a: storing a moved record back is E1001; q6b's trap was the probe's own `as u64` of a negative value) |
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

---

# Report (2026-11-01)

## 1. Forbidden: held

- Clean room: no C library's source or headers read (glibc, musl, the BSD
  libcs); no kernel source. glibc ran as a black box only (`g1`–`g5`, the
  differential's reference column). Each uapi file's package was checked
  with `dpkg -S` before its `#define` lines were read (all
  `linux-libc-dev`; `notes/lc02-evidence/uapi.log`). musl's and the BSD
  libcs' documentation, allowed as design references, was not needed.
  Every source is in `docs/SOURCES.md`.
- No build on the Mac: the Mac edits and commits; every build, probe and
  differential ran in hasu's `localhost/px75-ubuntu:2`, one block per
  run by both lock names, each job a single `flock -w 21600` per name,
  detached (`setsid -f`, a pid file, a log, an exit marker), and from
  2026-11-01 21:10Z on the emptier block. Every build ran from a commit
  (a worktree pinned to it; local WIP commits reached hasu as a git
  bundle, never pushed).
- kasumi untouched. Nothing under `/var/folders` touched. No attribution
  trailer, no label, merge, tag or close. Kills: none.

## R-list

- **R-lc02-1: the floating conversions.** (A) in this lane, against
  100,000 doubles per conversion; (B) lc03, this lane refusing them by
  name. **Recommended and implemented: B.** What (A) costs, measured: `f64`
  is refused on muzzle's only target (q3b: "a floating-point value (`f64`)
  on target x86_64-unknown-none (K10)"); `VaList`'s fields cannot be read
  (q3a: "cannot compile this yet — members on this type"); the one route
  left is to keep the record behind a `*VaList` and hand-walk
  `fp_offset`/`overflow_arg_area` at the documented offsets, which works
  on both tiers (q8) but duplicates the compiler's `va_arg`; and the
  conversion itself must be exact in integer arithmetic (a bignum over up
  to 1,074 fractional bits, correctly rounded, for `%f`, `%e`, `%g`, plus
  `%a`), held to glibc on 100,000 doubles per conversion. That is a lane's
  work on its own, after this one's streams and integer printf. An upstream
  ask would cut it: a `va_arg` that yields a double's bits as a `u64` on the
  none target.
- **R-lc02-2: how the refusal behaves.** (a) -1 with errno `EINVAL` and none
  of the call's output, the discipline glibc keeps for every failed call;
  (b) a trap naming the conversion (exit 134); (c) print the specification
  back as an unknown conversion. **Recommended and implemented: (a)**, a
  failure the program can see and test (`tests/own/printf_refusals.c`),
  named in `<stdio.h>`, the README and the ledger.
- **R-lc02-3: how closely streams follow glibc's flush moments.** (a) match
  every moment glibc was seen to choose (a full buffer written when the
  next byte arrives; the fill, block and remainder rule for a write too
  big; no room before the first byte; the 128-byte threshold; a
  line-buffered stream sending through its last newline; `st_blksize`
  buffers); (b) only what C promises, with cases that never look at a file
  mid-way. **Recommended and implemented: (a)**: the differential can then
  hold muzzle to glibc mid-stream (`stdio_buf.c`, `stdio_tty.c`,
  `stdio_exit.c`), and each choice is a ledger row.
- **R-lc02-4: positional arguments (`%n$`, `*m$`).** (a) refuse them now
  (-1, `EINVAL`), a later lane's; (b) implement here (two passes over the
  format to learn every argument's type). **Recommended and implemented:
  (a).**
- **R-lc02-5: how `exit` reaches the streams.** (a) `process` imports
  `stdio` and calls its flush; (b) a C-level hook. A bodyless `extern "c"
  fn` defined in wolf is E1306 on this target (`[abi.asm.roster]`: such a
  call may reach only the listed assembly or the hooks), so (b) would need
  an assembly trampoline. **Recommended and implemented: (a).**
- **R-lc02-6: the harness self-test in CI.** (a) this module PR leaves
  `.github/` alone, and `tests/difftest-selftest` runs in the lane's
  gauntlet; CI's `tools/difftest` already makes every new comparison; a
  follow-up adds the step (cross-cutting); (b) add it here and declare
  the PR cross-cutting. **Recommended and implemented: (a).**
- **R-lc02-7: `ungetc`'s depth.** (a) 8 bytes, last in first out (C
  guarantees one; glibc takes more); (b) unbounded, as glibc. **Recommended
  and implemented: (a)**, with a ledger row; no case pushes more than two.

## 3. Prediction, scored: 7 of 10, one in part

| # | predicted | measured | score |
|---|---|---|---|
| P1 | all 35 functions in wolf; new non-wolf code only `sys.S` storage | as predicted: `sys.S` gains the three stream slots, `stdin`/`stdout`/`stderr`, the stage, the termios and stat scratch, and the two texts | right |
| P2 | 7 new syscalls: read 0, close 3, fstat 5, lseek 8, ioctl 16, fcntl 72, openat 257 | those 7 (`unistd.lu`, `file.lu`, `char.lu`); `fstat` for `st_blksize`, `fcntl` for fdopen's access check | right |
| P3 | 13 or 14 new cases, 22–23 agreeing on both tiers | 14 (`unistd`, `stdio_open`, `stdio_char`, `stdio_ungetc`, `stdio_rw`, `stdio_buf`, `stdio_tty`, `stdio_err`, `stdio_exit`, `stdio_exit_`, `printf`, `printf_grammar`, `snprintf`, `vprintf`), 23 cases, plus 2 own cases | right |
| P4 | four new comparisons, each seen red by a self-test | `tests/difftest-selftest`: 14 checks, each plant red by name (files, stderr, stdout, exit) and each control green (stdin, tty, no tty, own) on both tiers | right |
| P5 | a corpus of over 300,000 calls, byte for byte | 629,764 calls (589,824 integer, 39,936 `c s p %` and their wide forms, 4 `%n` rows), 629,765 lines, identical on both tiers at the first run | right |
| P6 | 8 to 12 new ledger rows, plus the two refusals | 24 open-choice rows, 3 not-provided rows, 1 row where glibc departs from POSIX, 2 known differences not exercised | **wrong** (more rows, and one departure from the standard: fflush with pushback) |
| P7 | the plant red in `printf`, `printf_grammar` and `snprintf` | `snprintf.c` has no `%X`; see §4 for the plant's red cases | **wrong** in the list |
| P8 | 0 imports | `build: native: 8 objects, 57 exports defined, 43 internal references, 0 imports`; release the same | right |
| P9 | 1 to 3 wolf limitations to file | 2 (below) | right |
| P10 | R-lc02-1 = B; the refusal a -1 with errno EINVAL and none of the call's output | B, with that refusal to a stream; to memory the prefix stays, as for every failed call in glibc (the own case was first written to the wrong shape, red in the first full run, then corrected) | right in part |

Beyond the prediction:
- **glibc's flush moments are a model, not a list.** One rule set reproduces
  every number the probes saw. A buffer has no room until a byte has gone
  into it. A write that does not fit fills, flushes, sends whole buffer
  multiples straight, and keeps the remainder. A buffer under 128 bytes
  sends the whole rest. A line-buffered stream sends through its last
  newline. Among the numbers it reproduces are the odd ones: a first
  `fputs` into a 16-byte buffer reaching the file at once, and 4502 for a
  5000-byte line-buffered write with a newline at 4500.
- **One departure of glibc's from POSIX** (fflush with read-ahead and
  pushback both pending): muzzle follows POSIX, and that shape is an own
  case, not a differential one (`docs/LEDGER.md`).
- **Two of my own errors caught by the gates.** A reserved word (`take`)
  named a function: E0008 on the first compile that reached the parser.
  A macOS `sed` without `\b` then renamed only the definition, so one
  queued build (build5) was spent on a known failure. I also wrote the
  refusal's own case to a shape glibc's failures do not have, and it went
  red until corrected.

## 4. Evidence index

hasu, `localhost/px75-ubuntu:2` (clang 18.1.3, glibc 2.39), one block per
run. Logs under `notes/lc02-evidence/` (sha256 prefix):

| file | sha256 | shows |
|---|---|---|
| `explore1.log` | `5329d366568c…` | trunk `8d5896fa`: stale 5/5, build and difftest 9/9 on both tiers; probes q1–q5 round 1; g1, g2 |
| `explore2.log` | `7ee882d4b8dc…` | probes round 2 (q1, q2, q2b, q4, q6a, q6b, q7); g2 |
| `explore3.log` | `8aa3558fa8f4…` | q7, q8 on both tiers; g3, g4 (g4 under `ptyrun`) |
| `explore4a.log` | `4baa007794c6…` | g5 |
| `g1-glibc.out` … `g5-glibc.out`, `g4-glibc-tty.out` | `3f544ca839bc…`, `b2fd388d4d43…`, `86b10fc42ef1…`, `9d84fd0b35ee…`, `606e3c1bd863…` | glibc's answers, black box |
| `uapi.log` | `d73d32ad59f5…` | each uapi file's package (`linux-libc-dev`) and the numbers used |
| `red-cases-before-library-fea0707.log` | `03927e1d638b…` | **red first**: the 14 cases and the own case at `fea0707`, before any library code: `FAIL <case>: muzzle column does not build` on both tiers (44 such lines, the self-test's own columns included), every glibc column built, lc01's 9 cases green under the grown harness |
| `dev-first-full-run-c82c094.log` | `d0d7e939795e…` | a development state (a local WIP commit, never pushed): the first build, 0 imports on both tiers; self-test 14/14 on both; difftest 21 of 25 on both, the four reds being a throwaway smoke case, `stdio_buf` (the small line buffer), `stdio_ungetc` (glibc's fflush departure) and the own refusal case (its shape) |

The gauntlet at the head (`tests/stale-compiler`, `tools/build`,
`tests/difftest-selftest`, `tools/difftest`, both tiers, stopping at the
first nonzero step) and the plant's red run are in the PR body: they run at
the head this note is part of.

Probes and their scripts: `notes/lc02-probes/` (q1–q8 and their C
drivers, g1–g5, `ptyrun.c` as first written, the job scripts exactly as
they ran on hasu).

## 5. Done-when

- [x] Items 1–4: streams, the descriptor calls, the printf family without
  the floating conversions (R-lc02-1 = B), the differential grown to
  stdout, stderr, the exit status and written files, with stdin and a
  terminal; `<stdio.h>` and seven more headers hand-written; the archive
  self-contained (0 imports), no `libwolf_rt.a`.
- [x] The printf corpus from the grammar: 629,764 calls.
- [ ] The differential green on both tiers at the head, and CI green: the
  PR body and the hand-back.
- [ ] The plant (`%X` printing lower case), on `lc02-plant-X-lower`, red
  by name: the PR body and the hand-back.
- [x] README read and refreshed (a README-only commit); the ledger and
  sources updated.

## 6. Found

To file (not filed: a lane files nothing):
- **wolf-lang**: `VaList`'s fields cannot be read from wolf ("cannot
  compile this yet — members on this type (the std surface)", q3a), though
  `[abi.c.variadic.list]` documents them and `offset_of` names them.
- **wolf-lang**: on `x86_64-unknown-none` a variadic double cannot be
  taken at all (`va_arg[f64]` refused by K10, q3b); lc03 needs a way to
  take its bits (a `u64`), or it hand-walks a raw copy of the record (q8).
- **muzzle**: add `tests/difftest-selftest` to CI (a `.github/` change,
  cross-cutting; R-lc02-6).

To close: none.
