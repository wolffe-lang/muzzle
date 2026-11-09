# lc03: stdlib, time, errno and the POSIX file layer

Lane lc03 (the libc track's third layer, Opus). Base: muzzle trunk
`230cd000` (wolf 0.2.36). Branch `lc03`.

This file holds §2 as re-derived and §3 as committed before the first line
of library code. §1, §4 and §5 are filled in at the end.

## 2. Inputs, verified (re-derived 2026-11-09)

| input as written | at origin | drift |
|---|---|---|
| muzzle trunk `230cd000`, wolf 0.2.36 | `origin/trunk` = `230cd00012b6…`; `wolf-toolchain.toml` pins 0.2.36, rev `5adf3d91`, `wolf 0.2.36 (wolfgang, pin 5adf3d9)`, linux x86-64 `d383c74c8cd1…` | none |
| the pinned archive | release API digest `sha256:d383c74c8cd1…`, 82,827,649 bytes (read 1); `sha256sum` twice on the Linux test host of the staged copy (read 2): `d383c74c8cd1…`; `tools/fetch-toolchain` checks it a third time in every job | none |
| newer wolf | 0.2.37 and 0.2.38 are out; neither release note touches the freestanding target, the C membrane, C function pointers or floating point on `x86_64-unknown-none` | no pin move: this lane stays `module` |
| the build host | the Linux test host, in an Ubuntu container: clang 18.1.3, glibc 2.39, kernel 7.2.4, x86-64; `TZ` unset, `/etc/localtime` → `Etc/UTC`; `ulimit -c` unlimited and the core pattern a pipe to the host's handler; the job's directories on ext4 (`notes/lc03-evidence/explore1.log`) | see R-lc03-10 for the core pattern |
| the baseline at trunk | job `explore1` (block B): `tests/stale-compiler`, `tools/build` and `tools/difftest` on both tiers, every step exit 0, 25 of 25 cases on each | none |
| the licence | trunk already carries `LICENSE-EXCEPTION`, the muzzle Library Exception 1.0 (`7c95300`, muzzle#2): a GPLv3 section 7 permission modelled on the wolf Runtime Library Exception (`crates/wolf_rt/LICENSE-EXCEPTION` at wolf-lang `v0.2.36`, the file the release archives carry) and GCC's, for Independent Modules in any language, "statically or dynamically"; the README's licence section says the same in plain prose | the exception the lane was to add exists; R-lc03-2 |
| what lc02 left to this layer | the floating printf conversions (R-lc02-1 = B, "lc03 owns them" in the README and the ledger) | this lane's scope does not list them; R-lc03-12 |

### What wolf 0.2.36 gives this layer (probes `notes/lc03-probes/`, job `explore1`)

Each probe is built for `x86_64-unknown-none` on both tiers with
`--deny-warnings` and linked with trunk's `crt0.o` and `libmuzzle.a`.

| probe | question | answer, both tiers |
|---|---|---|
| p1 | can wolf define every C name this lane exports (58 names, `abs` … `readlink`, `environ`)? | yes: no name is a reserved word |
| p2 | a `#[repr(c)]` struct returned by value to C (`div_t`, `ldiv_t`) | yes: `div(-7, 2)` = -3, -1; `ldiv(-7000000000, 3)` = -2333333333, -1 |
| p2 | two listed assembly files | yes: `lib.asm-t1.o` and `lib.asm-t2.o` beside `lib.o` |
| p2 | a C function pointer called from wolf (no C function-pointer type: wolf-lang#520) | yes, through a three-instruction assembly trampoline that tail-jumps to it: a comparator answered -4, a `void (*)(void)` ran 3 times |
| p2 | a `char **` that C stored in assembly data, read from wolf | yes |
| p3a | a `f64` result on the freestanding target | **no**: "a floating-point value (`f64`) on target x86_64-unknown-none (K10: kernel code keeps no SSE or x87 state)" |
| p3b | a 128-bit integer for a 64 × 64 product | **no**: E0301, nothing named `u128` |

### glibc as the oracle (`g1`, black box only)

glibc 2.39's answers where the standards leave a choice open: `qsort` keeps
equal elements in their original order at every size probed (5 to
2,000,000 elements); `realloc(p, 0)` frees and answers a null pointer;
`aligned_alloc` with an alignment that is not a power of two fails with
`EINVAL`; `posix_memalign` answers `EINVAL` for an alignment below 8 or
not a power of two and leaves errno alone; `atexit` took 100 handlers;
`RAND_MAX` is 2147483647; `strerror` of a number no error has is
"Unknown error N", 41 and 58 among them; `perror` of a null or empty
prefix prints the text alone. strtod sets `ERANGE` when the exact value is
below `DBL_MIN` and the result inexact, even when it rounds up to `DBL_MIN`
(`2.2250738585072012e-308`); a NaN's parenthesised digits become its
payload (`nan(123)`, `nan(0x7ff)`). `gmtime_r` names its zone `GMT`,
`localtime_r` under `Etc/UTC` names it `UTC`; `mktime` with `tm_isdst` = 1
in a zone with no daylight time answers an hour earlier and clears it;
strftime prints an unsupported `%Ox` or `%+` back as written.

## 3. Prediction, committed before the first change

### The split (R-lc03-1)

The contract's scope is two layers' worth. The proposal: two PRs, merged in
order.

- **PR 1, branch `lc03`:** `<stdlib.h>` (the allocator's aligned forms,
  sorting and searching, the conversions, the arithmetic, the
  environment, `atexit` and `abort`, `rand`) and `<errno.h>` with
  `strerror` and `perror`. Its cases are in `tests/cases/`.
- **PR 2, branch `lc03b`, from PR 1's head:** `<time.h>`, the POSIX file
  layer and the three programs. Its cases wait in `tests/next/` (where
  `tools/difftest` does not look), and its first commit moves them into
  `tests/cases/`.

### The export list: 57 C functions and one data object

| header | functions | PR |
|---|---|---|
| `<stdlib.h>` (25) | `aligned_alloc` `posix_memalign` `qsort` `bsearch` `strtol` `strtoul` `strtoll` `strtoull` `strtod` `atoi` `atol` `atoll` `abs` `labs` `llabs` `div` `ldiv` `lldiv` `getenv` `setenv` `unsetenv` `atexit` `abort` `rand` `srand` | 1 |
| `<string.h>`, `<stdio.h>` (2) | `strerror` `perror` | 1 |
| data (1) | `environ` (`char **`, in `sys.S`, set by `_start` from the initial stack) | 1 |
| `<time.h>` (11) | `time` `clock_gettime` `nanosleep` `clock` `gmtime_r` `gmtime` `localtime_r` `localtime` `mktime` `timegm` `strftime` | 2 |
| `<fcntl.h>` `<unistd.h>` `<sys/stat.h>` `<dirent.h>` (19) | `openat` `stat` `fstat` `lstat` `unlink` `rename` `mkdir` `rmdir` `getcwd` `chdir` `opendir` `readdir` `closedir` `dup` `dup2` `pipe` `access` `symlink` `readlink` | 2 |

`open`, `read`, `write`, `close`, `lseek` and `isatty` are lc01's and
lc02's. `timegm`, `symlink` and `readlink` are beyond the contract's list:
`lstat` cannot be told from `stat` without a link to look at, and `timegm`
(POSIX.1-2024) is `mktime`'s UTC twin, used by the round-trip rows. `<errno.h>`
grows from 36 numbers to every one Linux defines: 133 names from the uapi
`asm-generic/errno-base.h` and `asm-generic/errno.h` (two of them the
aliases `EWOULDBLOCK` and `EDEADLOCK`), plus POSIX's `ENOTSUP` (= `EOPNOTSUPP`),
each equal to glibc's (`notes/lc03-evidence/uapi-errno.txt`).

### The cases and their expected output

20 new cases, each a C program written from the standards; their expected
output is glibc's, taken on the Linux test host (job `glibc1`: the case built as
`tools/difftest` builds it, run in a fresh directory with `TZ=UTC`), kept
whole in `notes/lc03-evidence/expected/` with `SHA256SUMS`:

| case | tests | lines | stderr bytes | exit | stdout sha256 |
|---|---|---|---|---|---|
| `stdlib_alloc` | `aligned_alloc` (13 alignments × 5 sizes, 4 bad alignments), `posix_memalign`, `realloc` keeping every byte through each class, mapped and aligned blocks, `calloc` | 121 | 0 | 0 | `2011992429a5…` |
| `stdlib_sort` | `qsort` over 19 sizes, element sizes 1, 3, 4, 24 and 100, ties in input order; `bsearch` | 77 | 0 | 0 | `3aac48d672a7…` |
| `stdlib_strtol` | 65 inputs × 13 bases × 4 functions, value, end and errno; `atoi` `atol` `atoll` | 924 | 0 | 0 | `242ce08f24bb…` |
| `stdlib_strtod` | 115 edge strings and two long ones; 3,837 exact halfway points (and just above and below), written out in full decimal by the case; 200,000 random decimal strings; 50,000 random hexadecimal ones | 138 | 0 | 0 | `a43bbe4b0ad7…` |
| `stdlib_misc` | `abs` `labs` `llabs`, `div` `ldiv` `lldiv`, `RAND_MAX` and rand's fixed properties | 24 | 0 | 0 | `ecb8fe2aa1f5…` |
| `stdlib_env` | `getenv` `setenv` `unsetenv` and `environ`, errors, 200 variables, an `environ` of the program's own | 35 | 0 | 0 | `ac09869bca65…` |
| `atexit_order` | handlers in reverse, one twice, streams flushed after them, `exit(3)` | 9 | 0 | 3 | `f21d4336bdaa…` |
| `atexit_return` | 42 handlers when `main` returns 7 | 44 | 0 | 7 | `e03e17e73923…` |
| `atexit_Exit` | `_Exit` runs no handler and flushes nothing | 1 | 0 | 5 | `96f56a26868e…` |
| `stdlib_abort` | SIGABRT, no handler, no flush | 1 | 55 | 134 | `9160d4be34c8…` |
| `errno_strerror` | all 134 names by value; `strerror` of -2…140 and seven far numbers; `perror` | 296 | 341 | 0 | `f9ea90a7940d…` |
| `time_clock` | `time`, `clock_gettime` (4 clocks, a bad one), `nanosleep` (and its errors), `clock` | 18 | 0 | 0 | `12bab9743a69…` |
| `time_conv` | `gmtime_r` `localtime_r` `timegm` `mktime` over 39 instants (to `LLONG_MIN` and `LLONG_MAX`), a daily sweep of 1900–2100, 36 normalizations | 221 | 0 | 0 | `ff9178a2af38…` |
| `time_strftime` | 70 conversions over 30 instants, `%z`/`%Z` by hand, 13 years × 11 year forms, the 12-hour clock, the count | 2242 | 0 | 0 | `3341ae242b85…` |
| `file_io` | `open` `openat` `read` `write` `close` `lseek` `dup` `dup2` `pipe` `isatty` `access` | 82 | 0 | 0 | `499245513d3f…` |
| `file_meta` | `stat` `fstat` `lstat` `unlink` `rename` `mkdir` `rmdir` `symlink` `readlink` | 58 | 0 | 0 | `ef0a56f5b996…` |
| `file_dir` | `getcwd` `chdir` `opendir` `readdir` `closedir`, 1,003 entries, a 255-byte name | 36 | 0 | 0 | `04fdf2bec27b…` |
| `prog_wc` | wc in plain C over stdin and three files it writes | 5 | 39 | 0 | `100a13ead45a…` |
| `prog_sort` | sort in plain C: bytewise, numeric and stable, reversed and unique, by field | 1148 | 0 | 0 | `c8b22b1924d6…` |
| `prog_tree` | a recursive lister in plain C: builds, walks (sorted, links not followed) and removes a tree | 72 | 0 | 0 | `42e5a5e7ecd8…` |

**Red first** (job `red1c`, block C, at `4d0389b`, `notes/lc03-evidence/red1c.log`):
every new case is `FAIL <case>: muzzle column does not build` on both
tiers, every glibc column builds, and the 23 trunk cases stay `PASS`.

### R-list (each with its recommendation, implemented unless ruled otherwise)

- **R-lc03-1, the split.** (a) one PR; (b) two, as above. **Recommend (b).**
- **R-lc03-2, the licence.** The exception is on trunk already (§2).
  (a) leave it: it is the wolf runtime's shape and names static linking;
  (b) reword it. **Recommend (a)**: no commit.
- **R-lc03-3, the allocator.** (a) keep lc01's: power-of-two classes over
  `brk`, then `mmap`, large blocks mapped alone; an aligned block is a third
  header kind, its second word the distance back to the block that carries
  it, so `free` and `realloc` find the carrier; (b) a new allocator.
  **Recommend (a).**
- **R-lc03-4, qsort's algorithm.** (a) a stable merge sort over a buffer
  from `malloc`, falling back to a stable in-place insertion sort when no
  buffer can be had, and a ledger row for the order of equal elements;
  (b) an in-place unstable sort. glibc's is stable at every size probed.
  **Recommend (a).**
- **R-lc03-5, strtod without a floating point type.** (a) the conversion in
  integers in wolf, correctly rounded for any input length (32-bit limbs in
  `u64` arithmetic; significant digits past 800 fold into a sticky bit,
  which cannot move a double's rounding), answering the bits; `strtod` is a
  four-instruction assembly entry that calls it and moves the bits to
  `%xmm0`; (b) wait for `f64` on the freestanding target. **Recommend (a).**
- **R-lc03-6, C function pointers** (qsort's comparator, atexit's handlers).
  (a) two tail-jump trampolines in `sys.S` (p2); (b) wait for wolf-lang#520.
  **Recommend (a).**
- **R-lc03-7, rand.** (a) muzzle's own generator (a 64-bit linear
  congruential state, its top 31 bits answered), glibc's `RAND_MAX`, and
  glibc's `srand(0)` = `srand(1)`; the values are never compared, since C17
  fixes no sequence; (b) glibc's sequence. **Recommend (a).**
- **R-lc03-8, the local zone.** (a) UTC always in this lane: `localtime_r`
  is `gmtime_r` with the zone named `UTC`, `mktime` is `timegm` with
  glibc's `tm_isdst` rule, `TZ` and `/etc/localtime` are not read, and
  `tools/difftest` runs both columns with `TZ=UTC`; (b) POSIX `TZ` strings
  now; (c) the zone database now. **Recommend (a)**, a ledger row; the
  database is a later lane's.
- **R-lc03-9, strerror's text.** The texts are glibc's own output, observed
  as a black box (`errno_strerror`'s expected output), generated into an
  assembly table (`src/errtext.S`; a `str` literal's bytes cannot reach C,
  wolf-lang#604). (a) that; (b) texts of the lane's own from the man pages.
  The contract asks for glibc's text, so **(a)**.
- **R-lc03-10, abort's core.** SIGABRT's default action dumps core, and on a
  host whose core pattern is a pipe the kernel hands the dump to that
  handler whatever `RLIMIT_CORE` says. (a) keep `stdlib_abort` in the
  differential and have `tools/difftest` set `ulimit -c 0`, so a handler
  that honours the limit stores nothing; (b) leave `abort` untested.
  **Recommend (a).**
- **R-lc03-11, PR 2's plant.** Both of the contract's plants (realloc,
  qsort) fall in PR 1. (a) PR 2 gets its own: `gmtime_r` taking 2100 for a
  leap year, red in `time_conv`; (b) none. **Recommend (a).**
- **R-lc03-12, printf's floating conversions.** lc02 left them to "lc03";
  this lane's scope does not list them. (a) they stay refused, by name, for
  a later lane, which can reuse strtod's big integers; (b) here.
  **Recommend (a).**

### Predicted, and what would falsify it

| # | predicted | falsified if |
|---|---|---|
| P1 | all 57 functions are wolf; new non-wolf code is assembly only: in `sys.S` the two trampolines, `strtod`'s entry, `environ` and storage (the atexit table, strtod's limbs, strerror's and the signal calls' scratch; in PR 2 the static `struct tm`), one line in `crt0.S` (storing `envp`), and one new listed file, `src/errtext.S` | a function needs C, or more assembly than listed |
| P2 | new system calls: PR 1 five (`rt_sigaction` 13, `rt_sigprocmask` 14, `getpid` 39, `gettid` 186, `tgkill` 234, all for `abort`); PR 2 seventeen (`stat` 4, `lstat` 6, `access` 21, `pipe` 22, `dup` 32, `dup2` 33, `nanosleep` 35, `getcwd` 79, `chdir` 80, `rename` 82, `mkdir` 83, `rmdir` 84, `unlink` 87, `symlink` 88, `readlink` 89, `getdents64` 217, `clock_gettime` 228) | another is needed |
| P3 | all 20 cases agree with glibc on both tiers at each PR's head (11 at PR 1's, 20 at PR 2's), and the 23 trunk cases stay green | any case disagrees on either tier |
| P4 | strtod's corpora (3,837 halfway strings, 200,000 decimal, 50,000 hexadecimal) agree digest for digest at the first run that builds | a digest line differs at the first run |
| P5 | the archive gate stays at 0 imports on both tiers | `nm -u` shows an import |
| P6 | `realloc`'s plant (the copy one byte short) is red in `stdlib_alloc` and lc01's `malloc` on both tiers; `qsort`'s (equal elements taken from the right run first) red in `stdlib_sort` only; PR 2's (`gmtime_r` taking 2100 for leap) red in `time_conv` only, and in `time_strftime` | another case goes red, or a named one stays green |
| P7 | 10 to 16 new ledger rows | fewer than 10 |
| P8 | wolf limitations met that a lane would file: 0 to 2 beyond the known (#520, #604, `f64` on the freestanding target) | more |
| P9 | the slowest case, `stdlib_strtod`, runs under 20 s on the native tier | it takes longer |

---

# Report, PR 1 (2026-11-09): stdlib, errno and strerror

## 1. Forbidden: held

- **Clean room.** No C library's source or headers were read (glibc, musl,
  the BSD libcs). glibc ran as a black box only: probes `g1`–`g3` and the
  differential's reference column. Each uapi file's package was checked
  with `dpkg -S` before its `#define` lines were read
  (`notes/lc03-evidence/red1c.log`). musl's and the BSD libcs'
  documentation, allowed as a design reference, was not needed. Every
  source is in `docs/SOURCES.md`.
- **Hosts.** Every build and differential ran on the Linux test host, in an
  Ubuntu container, under a block's two locks taken in the one order,
  each job detached with a pid file, a log and an exit marker, and built
  from a commit (a worktree pinned to it; local commits reached the host
  as a git bundle). Each job removes its worktree when it ends (the host's
  disk is short). Nothing was committed on the test host; nothing was
  written under `/tmp`.
- **Kills: four of the lane's own jobs, none started.** A duplicate of the
  red run, a run queued on the busier block, a run queued with a stale
  commit id, and a development run the gauntlet made redundant. (PR 1's
  head said three: the fourth came after it was written.) Each was still waiting on its block's lock. Its ancestry was
  checked, then its wrapper and its waiting `flock` were sent TERM. Their
  markers and logs were renamed `*.void-term`, with the reason recorded.
- No label, merge, tag or close; no attribution trailer.

## R-list, as ruled

- **R-lc03-1 = (b):** two PRs, this one (`lc03`) first, `lc03b` from its head.
- **R-lc03-2 = (a):** the exception on trunk is already the shape asked for;
  no commit.
- **R-lc03-3 to R-lc03-9, R-lc03-11:** as recommended in §3.
- **R-lc03-10 = (a):** `tools/difftest` sets `ulimit -c 0`.
- **R-lc03-12 = (a):** the floating conversions stay refused. The ledger's
  row, `<stdio.h>` and `format.lu` now say a later layer provides them.
- **R-lc03-13 (new): errno after glibc's first `perror`.** glibc's first
  `perror` on a stderr that has had no output yet leaves errno EINVAL,
  whatever it entered with; later calls keep errno (`g2`, `g3`, in
  forked children: it holds after `fflush(stderr)` or `setvbuf`, and not
  once stderr has had a byte of output). The likely cause, unobserved, is
  a write through a duplicate of fd 2 opened `"w+"`, which a write-only
  descriptor refuses. (a) muzzle leaves errno as it was; `errno_strerror.c`
  writes a line to stderr before its `perror` rows; a known-difference
  row in the ledger; (b) emulate it: a `perror` on a stream with no output
  yet ends with EINVAL when fd 2 is not open for reading. **Recommended
  and implemented: (a).** POSIX leaves errno after a successful call
  unspecified, and (b) would copy an artefact whose read-write branch was
  never observed.

## 3. Prediction, scored for PR 1

| # | predicted | measured | score |
|---|---|---|---|
| P1 | the functions in wolf; new non-wolf code: `sys.S`'s two trampolines, `strtod`'s entry, `environ` and storage, one line of `crt0.S`, `errtext.S` | as predicted. `strtod`'s entry is five instructions, not four (`sub`, `call`, `add`, `movq`, `ret`) | right |
| P2 | PR 1's new system calls: `rt_sigaction` 13, `rt_sigprocmask` 14, `getpid` 39, `gettid` 186, `tgkill` 234 | those five, all in `abort` | right |
| P3 | the 11 cases agree on both tiers at PR 1's head, the trunk cases stay green | at the first run that built (`dev3`), 10 of the 11 agreed on both tiers; `errno_strerror` differed on one line, glibc's first `perror` leaving errno EINVAL (R-lc03-13). The case now writes stderr first, so every case agrees at the head (the gauntlet) | **wrong in part** |
| P4 | strtod's corpora agree digest for digest at the first run that builds | `dev3`, both tiers: 3,837 halfway strings, 200,000 decimal, 50,000 hexadecimal, and the 117 single rows, all identical | right |
| P5 | 0 imports on both tiers | `tools/build`'s gate passes on both tiers (it refuses any import) | right |
| P6 | realloc's plant red in `stdlib_alloc` and `malloc`; qsort's in `stdlib_sort` only | see the PR body and the hand-back: the plants run at the head this note is part of | (at the head) |
| P7 | 10 to 16 new ledger rows | 18 open-choice rows, 3 known differences, 3 new link rows (one replaced) | **wrong** (more) |
| P8 | 0 to 2 wolf limitations beyond the known ones | none to file. Four rules were met and kept: `borrow` is a keyword; a constant is module-wide across a module's files (E0302); `.addr()` needs `unsafe` (E1301); a `pub fn` that is not an export cannot take a raw pointer (E1302) | right |
| P9 | `stdlib_strtod` under 20 s on the native tier | not measured alone: difftest times no single case | not measured |

Beyond the prediction:
- **glibc's `perror` and errno**, above (R-lc03-13).
- **The corpora agreed at the first run that built.** No strtod row needed
  a second attempt. That run used the halfway strings, the tiny-value
  rule (ERANGE below `DBL_MIN` when inexact, `g1`) and the NaN payloads
  taken from `g1` before any code was written.
- **The first two runs did not compile.** They failed on wolf's rules
  above: a duplicate constant, the `borrow` keyword, eleven `.addr()`
  reads outside `unsafe` and one `pub fn` with a raw pointer. Those runs
  are `dev1` and `dev2`; their logs are not kept.

## 4. Evidence index (PR 1)

The Linux test host's container: clang 18.1.3, glibc 2.39. Logs under
`notes/lc03-evidence/` (sha256 prefix; paths in the first lines are the
lane's directory, written `<lane>`):

| file | sha256 | shows |
|---|---|---|
| `explore1.log` | `e31ae51ff9cc…` | trunk `230cd000`: stale, build and difftest exit 0 on both tiers, 25 of 25; probes p1–p3b; `g1` |
| `red1c.log` | `a2cf88560a41…` | **red first** at `4d0389b`: every new case `FAIL <case>: muzzle column does not build` on both tiers, every trunk case `PASS`; the uapi numbers |
| `glibc1.log`, `expected/` | `dafbb597f31c…`, `expected/SHA256SUMS` | glibc's expected output for the 20 cases |
| `uapi-errno.txt` | `dea59181197f…` | the 133 errno names and numbers, as read |
| `dev3-first-build.log` | `049c8cfe86ab…` | the first run that built (`fcdffb1`): 12 of 13 named cases agree on both tiers, `errno_strerror` one line apart |
| `g2-glibc.out`, `g2-glibc.err`, `g3-glibc.out` | `9f6391e1836c…`, `767aadc80786…`, `275c3a146fd6…` | glibc's `perror` and errno |

The gauntlet at the head (`tests/stale-compiler`, `tools/build`,
`tests/difftest-selftest`, `tools/difftest`, both tiers, stopping at the
first nonzero step) and the two plants' records are local records, named
in the PR body and the hand-back.

## 5. Done-when (PR 1)

- [x] The prediction first; the rows red before the library (`red1c`).
- [x] `<stdlib.h>`: the aligned allocation forms, `qsort` and `bsearch`,
  the strtol family, `strtod`, the integer arithmetic, the environment,
  `atexit` with `exit`, `abort`, `rand` and `srand`.
- [x] `<errno.h>` with every number Linux defines; `strerror` with glibc's
  text; `perror`.
- [x] The ledger, the sources and the README.
- [ ] The gauntlet green at the head, and the two plants red by name: the
  PR body and the hand-back.

---

# Report, PR 2 (2026-11-09): time, the file layer and the programs

Branch `lc03b`, from PR 1's head `782c3e1`. Its first commit moves the
nine cases from `tests/next/` into `tests/cases/`, so they are red by name
at that commit (no `<time.h>`, `<sys/stat.h>` or `<dirent.h>` yet), as
`red1c` showed them before any library code.

## 1. Forbidden: held

As for PR 1: no C library's source or headers read; glibc a black box;
every build on the Linux test host from a commit, under a block's two
locks; nothing under `/tmp`; no commit on the test host; no kill in this
part; no label, merge, tag or close; no attribution trailer.

## 3. Prediction, scored for PR 2

| # | predicted | measured | score |
|---|---|---|---|
| P1 | the functions in wolf; the new non-wolf code is `sys.S` storage (the static struct tm and its texts) | as predicted: the struct tm, a timespec, the day and month names, `GMT` and `UTC` | right |
| P2 | seventeen new system calls | those seventeen: stat 4, lstat 6, access 21, pipe 22, dup 32, dup2 33, nanosleep 35, getcwd 79, chdir 80, rename 82, mkdir 83, rmdir 84, unlink 87, symlink 88, readlink 89, getdents64 217, clock_gettime 228 | right |
| P3 | the 9 cases agree on both tiers | at the first run that built (`dev8`), 8 of 9 agreed on both tiers; `time_conv` trapped (muzzle's own overflow: `floor_div(t, 86400) * 86400` leaves i64 for t near `LLONG_MIN`). Fixed by truncating and stepping down; then 45 of 45 cases on both tiers (`dev9`) | **wrong in part** |
| P6 | the leap plant red in `time_conv` | see the PR body and the hand-back | (at the head) |
| P8 | no new wolf limitation | none: `shared` is a keyword, renamed | right |

strftime agreed with glibc at its first build, on every one of its 2,242
lines: the widths, the negative years and the specifications copied back
had been read from glibc's expected output before the code was written.

## 4. Evidence index (PR 2)

| file | sha256 | shows |
|---|---|---|
| `dev8-first-build-part2.log` | `efcc384abb00…` | the first build of this part: 8 of the 9 cases agree on both tiers, `time_conv` traps at its `LLONG_MIN` row |

The red rows are `red1c`'s (PR 1's evidence, every one of these nine
cases `FAIL <case>: muzzle column does not build`). The gauntlet at this
PR's head and its plant's record are local records, named in the PR body
and the hand-back.

## 5. Done-when (PR 2)

- [x] `<time.h>`: the clocks, the calendar (UTC), strftime.
- [x] The POSIX file layer and directory streams.
- [x] Three programs in plain C (`prog_wc`, `prog_sort`, `prog_tree`)
  agree with glibc on both tiers.
- [x] The ledger, the sources and the README.
- [ ] The gauntlet green at the head and the plant red by name: the PR
  body and the hand-back.

---

# PR 3 (2026-11-09): the earlier notes in plain words

muzzle is public, so its tracked prose names no host, no lane or home
path, no planning path, no private repository and no ruling or STATUS
number. This PR rewrites what lc01, lc02 and the mz lanes wrote before
that rule, and the comments elsewhere that carried ruling numbers:

- the prose notes (`notes/lc01-the-first-layer.md`, `notes/lc02-stdio.md`,
  `notes/mz-ffi.md`, `notes/mz2-…` to `notes/mz6-…`, the lc01 probes'
  README): the Linux test host, the Mac, the lane's contract, the lane's
  own directory, and each ruling said in words;
- the evidence and probe files no note cites by digest: a mechanical
  substitution (`<lanes>/`, `<home>/`, `<user>`, `test-host`);
- `README.md`, `docs/SOURCES.md`, `docs/LEDGER.md`, the headers'
  comments, `src/sys.S`, `src/sys/sys.lu`, `src/wolf.pkg`,
  `tools/difftest` and `tests/own/printf_refusals.c`: comments only.

**Left byte for byte:** every evidence file a note or a digest list cites
by digest (17 of them carry a host name or a path), so each cited digest
still matches its file; and the CI files (`.github/workflows/ci.yml`,
`ci/`) and the pin (`wolf-toolchain.toml`), whose comments are a
cross-cutting change of their own.

This prediction was written after the edits were drafted in the working
tree and before any of them was committed or built.

| # | predicted | falsified if |
|---|---|---|
| Q1 | no compiled byte moves: the gauntlet is green with 45 of 45 cases on both tiers, as at `de3eeb5` | any step is nonzero, or a count differs |
| Q2 | the cited files are unchanged: each digest a note cites still matches | a cited file differs from trunk |
| Q3 | after this PR, the tracked tree's hits for host names, lane and home paths, planning paths and private repository names are only in the cited files, the CI files and the pin | a hit elsewhere |
| Q4 | a plant (`<stddef.h>`'s rewritten comment left unterminated) is red by name: every case's muzzle column fails to build, on both tiers | a case still builds |
