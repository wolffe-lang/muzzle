# mz2: muzzle at wolf 0.2.29, and the stale-compiler build (muzzle#4)

Lane mz2 (wave 53, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz2-muzzle-at-0229.md` (planning commit `42ae4fb`,
authored 2026-10-13). Two things, each in its own commits: the pin
(wolf 0.2.29, lupin 0.1.52), and muzzle#4 (`tools/build` uses the pin's
compiler or fails loudly), with a test seen red first. This file holds
the inputs as re-derived and the prediction, committed before the first
change; the evidence and the score follow.

## 1. Forbidden

Nothing changes beyond the pin, what the pin forces, `tools/build`, its
test and the CI step that runs it. The licence exception text is not
touched. No C library's source is read. Toolchains come from the release
archives by digest, each digest read twice. No build on nomad-1; kasumi
only, under `~/lanes/mz2/`. No merge, no tag, no issue filed or closed.
No attribution trailers.

## 2. Inputs, verified (2026-10-13)

| input | at origin |
|---|---|
| muzzle trunk | `47e9285` (pin wolf 0.2.28, `f71ece8e`, linux x86-64 `3215f321…`); no drift from the contract |
| wolf 0.2.29 | tag `v0.2.29` → `6ffe7c6056ee…` (wolf-lang); release published 2026-10-12T23:22:55Z, not a draft; asset `wolf-0.2.29-x86_64-unknown-linux-gnu.tar.gz`, 78,902,948 bytes, sha256 `ae466e85418a48758460a981c9fab690462f779dae967cface86c555e649d7c1` (read once from the release API's `digest`, once by `sha256sum` of the download on kasumi). `wolf --version`: `wolf 0.2.29 (wolfgang, pin 6ffe7c6)` |
| lupin 0.1.52 | tag `v0.1.52` → `025428b725a3…` (wolf-interp); release published 2026-10-12T17:20:49Z; asset `lupin-0.1.52-x86_64-unknown-linux-gnu.tar.gz`, 2,943,200 bytes, sha256 `f5194b74879c45ad96484566950b4ae59fe1faa7ccd86461c6656f9e662b698f` (read the same two ways). `lupin --version`: `lupin 0.1.52 (wolf-interp, reference interpreter at pin 294d626)` |
| muzzle#4 | open; text as filed by mz. Its "why" is re-derived in §3 (P5), not taken |
| mz (PR #3) | merged `47e9285`; its §6 is where #4 comes from |
| bump notes read | 0.2.29's "read this before you bump" at `v0.2.29:CHANGELOG.md` |

What 0.2.29 says that can reach muzzle (freestanding, no std, no runtime
linked, no host builtin, no task, no signal):

- s231 (the pool), s232 (signals), s235 (`wolf lsp`): the runtime, five
  hosted builtins and the language server. "Every hosted binary's bytes
  move (`libwolf_rt.a`)": muzzle links neither `libwolf_rt.a` nor
  `libwolf_rt_none.a`.
- Five new prelude names (`os_signal_ignored`, `os_signal_send`,
  `os_signal_number`, `os_pid`, `os_ppid`); W0304 under
  `--deny-warnings` if a module declares one. The release's own grep
  read muzzle at `7c95300`; re-derived here at `47e9285`: no match.
- The release's downstream line for muzzle (the `ffi` line, "muzzle#3 is
  open for it") is stale: PR #3 merged it on 2026-10-12.

Two contract lines that do not fit the repo as found, reported and not
absorbed:

- **lupin is not pinned by any muzzle file**, and no muzzle tool runs
  it (`wolf-toolchain.toml`'s header says why). This lane takes lupin
  0.1.52 from its archive by digest and records what it answers for
  muzzle (P8); whether that makes a `[lupin]` table in the pin file is
  decided by what it answers.
- **The release archive ships no separate `conform-run`**; the checked
  machine is `wolf conform-run --json --checked`, a subcommand of the
  archive's `wolf`.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.29 `tools/build` passes on both tiers: native 6 objects, release 4, 19 exports, 0 imports, as at 0.2.28 | any error or warning, another object count, or the gate fails |
| P2 | `tools/difftest` 9/9 on both tiers; every case's exit status and line count equal to 0.2.28's | any case differs |
| P3 | **the pin moves no output**: every object of both tiers, debug info stripped, is byte-identical to what 0.2.28 built from the same source (0.2.29 changed the runtime, hosted builtins and the language server, none of which muzzle reaches) | any stripped object differs |
| P4 | the pin forces two edits and no other: `wolf-toolchain.toml` and README's `wolf 0.2.28` build line | a source, test or header edit is needed |
| P5 | #4 as filed is right about the cause: with trunk's `tools/build`, a compiler directory that sorts before the pin's is the one that runs. A test that stages one (identifying as an older version) has **4 red cases of 5** at trunk's `tools/build`: stale beside the pin (the stale one runs), stale alone (the build goes ahead, exit 0), the pin's directory holding another version (goes ahead), `WOLF=` naming a stale one (goes ahead); the control (the pin alone) is green | another count, or a red for a reason other than the stale compiler having run |
| P6 | after the fix the same test is 5 of 5; a refusal names the version line found and the one the pin wants, on stderr, exit non-zero, and the stale compiler's `build` is never invoked | otherwise |
| P7 | CI: the plant (the test on trunk's `tools/build`, a branch of its own) is red on both jobs at the test's step; the head is green on both | otherwise |
| P8 | muzzle's tests run on **native and release only**. The checked machine (`wolf conform-run --json --checked src/muzzle.lu`) and lupin 0.1.52 each **refuse muzzle by name** (the C membrane or the listed assembly), neither runs it, neither crashes | either runs it, or dies without naming a reason |
| P9 | no muzzle gate reads `wolf build`'s exit 2 as a skip: with a compile error planted in `src/`, `tools/build` exits 2 on both tiers, `tools/difftest` after it exits 1 with 9 FAIL and 0 PASS, and no line says skip | a zero exit anywhere, or a skip |

## 4. Evidence (kasumi, linux x86-64, clang 23.1.1, glibc 2.44)

Each log is the raw output of a script at the named commit, every exit
status echoed bare (no pipe on a command under test), each run from a
tree with no `.wolf-bin/`, `build/`, `out/` or `src/.lu-cache/`.
`notes/mz2-evidence/`:

| log | commit | what |
|---|---|---|
| `baseline-0.2.28.log` | `47e9285` (trunk) | wolf 0.2.28: build 6 and 4 objects, 19 exports, 0 imports; difftest 9/9 on both tiers |
| `pin-0.2.29.log` | `f6e2175` (the pin) | wolf 0.2.29 by digest: the same lines, the same exit statuses and line counts case for case |
| `objects-0.2.28-0.2.29.log` | `47e9285` against `f6e2175` | 12 objects (6 native, 4 release, `crt0.o` twice), `objcopy --strip-debug`, byte for byte: 12 same, 0 differ. Control that the compare fires: unstripped, the 5 native wolf objects differ (DWARF names the compiler). Release objects carry no DWARF, so the control does not reach that tier |
| `stale-real-trunk-build.log` | `f6e2175` (trunk's `tools/build`) | **#4 with real compilers**: 0.2.28's directory beside 0.2.29's, `tools/fetch-toolchain` just run for 0.2.29, `tools/build` exit 0 on both tiers, and every native object's `DW_AT_producer` is `wolf v0 (wolfgang 0.2.28)` |
| `stale-test-red-b94be0d.log` | `b94be0d` (the test, no fix) | `tests/stale-compiler`: pass=1 fail=4 on both tiers, exit 1; each failure is "the build ran the stale compiler", with the build's exit 0 and the stale compiler's own command line |
| `stale-test-green.log` | `dc3bcac` (the fix) | the same test file (blob `344d7be3` in both logs): pass=5 fail=0 on both tiers, exit 0 |
| `stale-real-fixed.log` | `dc3bcac` | real compilers again: 0.2.28 alone → exit 2, "the pin wants 'wolf 0.2.29 (wolfgang, pin 6ffe7c6)' … found 'wolf 0.2.28 (wolfgang, pin f71ece8)' … not used", nothing built; 0.2.28 beside 0.2.29 → built by 0.2.29 (producer line); `WOLF=` the real 0.2.28 → exit 2 naming both |
| `green-fix-dc3bcac.log` | `dc3bcac` | the whole build and difftest again with the fixed `tools/build`: 6 and 4 objects, 9/9 on both tiers |
| `machines-0.2.29-0.1.52.log` | `dc3bcac` | what each machine answers for the package (below) |
| `exit2-is-a-failure.log` | `dc3bcac` + a planted type error, never committed | `wolf build` exit 2 (E0401); `tools/build` exit 2 on both tiers, 0 objects left; `tools/difftest` after it pass=0 fail=9, exit 1; `tests/stale-compiler` exit 1; no line in any gate or the workflow says skip |

CI (`.github/workflows/ci.yml`, ubuntu x86-64, glibc 2.39, clang 18):

- the plant, on a branch of its own: `mz2-plant` = trunk `47e9285` plus
  the test (`90630d9`) and its CI step (`285ff7c`), **no fix, trunk's
  pin (0.2.28)**. Run 38478997938 (workflow_dispatch): both jobs fail at
  the step `the stale compiler`, pass=1 fail=4, every failure "the build
  ran the stale compiler" under `tools/build exit=0`.
- the same test on this branch before the fix, at the 0.2.29 pin: head
  `b94be0d`, run 38479030166, both jobs fail at the same step with the
  same four lines.
- the fix: head `dc3bcac`, run 38479139294, both jobs green.

### The machines

| machine | command | answer for muzzle |
|---|---|---|
| native | `wolf build muzzle.lu --target x86_64-unknown-none --emit=obj --deny-warnings` (`tools/build native`) | builds; **the tests run here** (9/9) |
| release | the same with `--release` (`tools/build release`) | builds; **the tests run here** (9/9) |
| checked | `wolf conform-run --json --checked muzzle.lu` | **refused by name**: `verdict: unsupported`, "a link-time symbol (`extern "c" let`)", span 653..689, phase reached `mem`; exit 0 (a record, not a run) |
| lupin 0.1.52 | `lupin run muzzle.lu` | **refused by name**: "unsupported: a link-time symbol (`extern "c" let __muzzle_bins`) … defined by the image's link, which this machine does not model ([abi.link.extern])"; exit 4 |

`wolf run --checked muzzle.lu` is the native build (not the checked
machine) and answers exit 4, "no `main` function": muzzle is a library.
`conform-run --native` and `--release` answer `unsupported` for the same
reason; the two compiling tiers reach muzzle only through `wolf build
--emit=obj` for the freestanding target. muzzle's tests are C programs
linked against the archive, so they can run on the two compiling tiers
and nowhere else. No `[lupin]` table was added to `wolf-toolchain.toml`:
lupin 0.1.52's archive was taken by digest and asked, it answers
`unsupported`, and no muzzle tool runs it, so a pin there would be a
line nothing reads.

### How the stale compiler was picked up, and what stops it now

`tools/fetch-toolchain` unpacks the pinned archive into
`.wolf-bin/wolf-<version>-<host>/` and removes only that one directory
first. Trunk's `tools/build` chose its compiler with
`ls .wolf-bin/wolf-*/wolf | head -1`: the first name in sort order, and
it never asked the compiler what it was. `wolf-0.2.28-…` sorts before
`wolf-0.2.29-…`, so after a pin move on a checkout that had built
before, the old compiler ran and the build was green.
`stale-real-trunk-build.log` shows exactly that at this lane's own pin
move.

`tools/build` now reads `version` and `version_line` from
`wolf-toolchain.toml`, takes the driver from the pin's own directory
(or from `$WOLF`), and compares the first line of its `--version` with
`version_line` before it compiles anything. A missing directory or
another version is exit 2 with both version lines on stderr; other
directories in `.wolf-bin/` are named and not used.

## 5. Score: 9 of 9 (P7's green half is read at the PR's head)

| # | result |
|---|---|
| P1 | **exact.** 6 and 4 objects, 19 exports, 0 imports, no warning |
| P2 | **exact.** 9/9 on both tiers; the two logs differ only in their version lines |
| P3 | **exact. The pin moves no output**: 12 of 12 stripped objects byte-identical; every case's stdout, exit status and line count unchanged |
| P4 | **exact.** `wolf-toolchain.toml` and README's build line; no source, header or case edit |
| P5 | **exact.** 4 red of 5 at trunk's `tools/build`, on both tiers, on kasumi and in CI, each for the stale compiler having run |
| P6 | **exact.** 5 of 5; each refusal names both version lines; the stale compiler never compiled |
| P7 | **exact** for the red half (runs 38478997938 and 38479030166); the green half is the PR's head |
| P8 | **exact.** Both refuse by name, for the C membrane's link-time storage; neither runs muzzle, neither crashes |
| P9 | **exact.** Exit 2, then exit 1 with 9 FAIL; no skip path exists in `tools/` or the workflow (the only `if:` is the artifact upload's `always()`) |

## 6. Found on the way (not changed here, not filed)

- **`$WOLF` is held to the pin too.** Before, `WOLF=` any driver built
  with it. Now a `$WOLF` whose version line is not the pin's is refused
  like any other stale compiler. Building with another version means
  moving the pin file (as each pin commit does). If an escape hatch is
  wanted, it is a decision, not something this lane added.
- **`wolf build` writes `src/.lu-cache/`** (interfaces, modules,
  objects) and `.gitignore` does not list it, so a build leaves the tree
  dirty. It existed before this pin. This lane's evidence scripts remove
  it before every run so that no object is a cached one, and
  `tests/stale-compiler` removes it from each scratch copy.
- `tools/fetch-toolchain` still leaves older directories in
  `.wolf-bin/`. They are no longer picked up; they cost disk only.
- wolf-lang's 0.2.29 CHANGELOG says of muzzle "`capabilities: [ffi]` …
  owed since 0.2.27 (muzzle#3 is open for it)". PR #3 merged on
  2026-10-12; the line was stale when the release was cut.
- `wolf conform-run` exits 0 with `verdict: unsupported`. No muzzle gate
  reads it; one that did would have to read the verdict, not the status.
- mz's finding stands: `docs/LEDGER.md`'s #598 row and the heap comment
  describe 0.2.24 behaviour that 0.2.25 fixed.

Inferred, not verified: that a release-tier object was written by the
compiler the log says (release objects carry no DWARF producer; the
test's invocation log is the evidence for that tier). Not run: any host
but x86-64 Linux (kasumi, and ubuntu in CI); the release archives for
other hosts were not downloaded, and only the linux x86-64 digests were
read twice.
