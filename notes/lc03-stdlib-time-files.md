# lc03: stdlib, time, errno and the POSIX file layer

Lane lc03 (the libc track's third layer, Opus). Base: muzzle trunk
`230cd000` (wolf 0.2.36). Branch `lc03`.

This file holds §2 as re-derived and §3 as committed before the first line
of library code. §1, §4 and §5 are filled in at the end.

## 2. Inputs, verified (re-derived 2026-11-09)

| input as written | at origin | drift |
|---|---|---|
| muzzle trunk `230cd000`, wolf 0.2.36 | `origin/trunk` = `230cd00012b6…`; `wolf-toolchain.toml` pins 0.2.36, rev `5adf3d91`, `wolf 0.2.36 (wolfgang, pin 5adf3d9)`, linux x86-64 `d383c74c8cd1…` | none |
| the pinned archive | release API digest `sha256:d383c74c8cd1…`, 82,827,649 bytes (read 1); `sha256sum` twice on hasu of the staged copy (read 2): `d383c74c8cd1…`; `tools/fetch-toolchain` checks it a third time in every job | none |
| newer wolf | 0.2.37 and 0.2.38 are out; neither release note touches the freestanding target, the C membrane, C function pointers or floating point on `x86_64-unknown-none` | no pin move: this lane stays `module` |
| the build host | hasu, `localhost/px75-ubuntu:2`: clang 18.1.3, glibc 2.39, kernel 7.2.4, x86-64; `TZ` unset, `/etc/localtime` → `Etc/UTC`; `ulimit -c` unlimited and the core pattern a pipe to the host's handler; the job's directories on ext4 (`notes/lc03-evidence/explore1.log`) | see R-lc03-10 for the core pattern |
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
output is glibc's, taken on hasu (job `glibc1`: the case built as
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
