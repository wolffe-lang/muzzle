# mz3: muzzle at wolf 0.2.30

Lane mz3 (crawl 54, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz3-muzzle-at-0230.md` (planning commit `be08902`,
authored 2026-10-16). One deliverable: the pin (wolf 0.2.30, lupin
0.1.53) in its own commit, build and tests green on both tiers, every
compiled object compared with 0.2.29's; and the finding mz2 left,
`src/.lu-cache/` into `.gitignore`. This file holds the inputs as
re-derived and the prediction, committed before the first change; the
evidence and the score follow.

## 1. Forbidden

Nothing changes beyond the pin, what the pin forces, and the one
`.gitignore` line. The licence exception text is not touched. No C
library's source is read. Toolchains come from the release archives by
digest, each digest read twice. No build on nomad-1; kasumi only, under
`~/lanes/mz3/`. No merge, no tag, no issue filed or closed. No
attribution trailers.

## 2. Inputs, verified (2026-10-16)

| input | at origin |
|---|---|
| muzzle trunk | `44387d0` (pin wolf 0.2.29, `6ffe7c6`, linux x86-64 `ae466e85…`); no drift from the contract |
| wolf 0.2.30 | tag `v0.2.30` (tag object `ffb7c5b3`) → `1b37f368228d…` (wolf-lang); release published 2026-10-15T08:53:05Z, not a draft; asset `wolf-0.2.30-x86_64-unknown-linux-gnu.tar.gz`, 79,910,152 bytes, release API digest sha256 `4e55b39ed243d3e8ed3be24f716186529e1942935464476d47c4bc983329fe89` (read 1; read 2 is `sha256sum` of the download on kasumi, §4) |
| lupin 0.1.53 | tag `v0.1.53` (tag object `83631915`) → `7896eddc17fd…` (wolf-interp); release published 2026-10-15T03:01:38Z; asset `lupin-0.1.53-x86_64-unknown-linux-gnu.tar.gz`, 2,945,862 bytes, release API digest sha256 `5db5533236578008625885c7e3ed0a21084b8fa064219808ae668dedb0edccaa` (read 1; read 2 on kasumi). 0.2.30's `PAIRING` names the same `5db55332…` |
| bump notes read | 0.2.30's "read this before you bump" at `v0.2.30:CHANGELOG.md` |

What 0.2.30 says that can reach muzzle (freestanding, no std, no
runtime linked, no host builtin, no proc, no terminal):

- s233 (E1010, procs and regions), s234 (terminal bits), s236 (ids),
  the bundled std: none reachable; muzzle spawns no proc, uses no
  region, links no runtime and imports no std.
- Eleven new prelude names; the release's own grep read muzzle at
  `44387d08` and found no declaration. Its downstream line: "muzzle:
  the pin; it uses none of the new names."
- **s755 (#755)**: "s755 changes how a straight-line function's locals
  are lowered", for a function with no `if`, loop, call or index. This
  is the one change that could reach muzzle's objects. muzzle's 691
  lines of wolf have 38 narrowing `as` casts and no `str` comparison;
  read by eye, every function holding a cast also holds a call, a loop,
  an `if` or an index, so none is the straight-line shape.

Carried from mz2 and still true: no muzzle file pins lupin and no muzzle
tool runs it, so lupin 0.1.53 is taken by digest only to record what it
answers (P6); no `[lupin]` table is added.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.30 `tools/build` passes on both tiers: native 6 objects, release 4, 19 exports, 0 imports, as at 0.2.29 | any error or warning, another object count, or the gate fails |
| P2 | `tools/difftest` 9/9 on both tiers; every case's exit status and line count equal to 0.2.29's | any case differs |
| P3 | **the pin moves no output**: all 12 objects (6 native, 4 release, `crt0.o` on each), debug info stripped, byte-identical to 0.2.29's from the same source. The compare fires: unstripped, the 5 native wolf objects differ (their DWARF names the compiler) | any stripped object differs, or the unstripped control shows no difference |
| P4 | the pin forces two edits and no other: `wolf-toolchain.toml` and README's `wolf 0.2.29` build line | a source, test or header edit is needed |
| P5 | mz2's pin check, at its first real use, on a reused checkout with 0.2.29 still staged in `.wolf-bin/`: after the pin moves and before `tools/fetch-toolchain`, `tools/build` refuses with exit 2, naming 0.2.29 found and 0.2.30 wanted; after the fetch it builds with 0.2.30 (native objects' DWARF producer says 0.2.30). `tests/stale-compiler` 5 of 5 on both tiers | a build with 0.2.29 after the pin moves, or the test below 5 of 5 |
| P6 | the checked machine (`wolf conform-run --json --checked`) and lupin 0.1.53 each refuse muzzle by name (the link-time symbol), as at 0.2.29 / 0.1.52 | either runs it, or dies without naming a reason |
| P7 | after a build, trunk's tree is dirty with `src/.lu-cache/`; with the `.gitignore` line, `git status --porcelain` is empty after both tiers' build and difftest | anything else listed |
| P8 | CI: the plant (the 0.2.30 pin with one digest character changed, a branch of its own) is red on both jobs at the fetch step; the head is green on both | otherwise |
| P9 | no muzzle gate reads `wolf build`'s exit 2 as a skip at 0.2.30: with a compile error planted in `src/` (never committed), `tools/build` exits 2 on both tiers | a zero exit, or a skip |

## 4. Evidence (kasumi, linux x86-64, clang 23.1.1, glibc 2.44)

Each log is the raw output of the script beside it in
`notes/mz3-evidence/scripts/`, every exit status echoed bare (no pipe on
a command under test). `run.sh` starts each tree with `git clean -xfd`
(no `.wolf-bin/`, `build/`, `out/` or `src/.lu-cache/`).

| log | commit | what |
|---|---|---|
| `baseline-0.2.29.log` | `44387d0` (trunk) | wolf 0.2.29: build 6 and 4 objects, 19 exports, 0 imports; difftest 9/9 on both tiers; producers `wolfgang 0.2.29`; after the run `git status` lists `?? src/.lu-cache/` |
| `pin-0.2.30.log` | `074a349` | wolf 0.2.30: the same counts, 9/9 on both tiers, every case's exit status and line count as at 0.2.29; producers `wolfgang 0.2.30`; after the run `git status --porcelain` is empty |
| `objects-0.2.29-0.2.30.log` | both | 12 objects stripped (`objcopy --strip-debug`): 12 same, 0 differ. Control: unstripped, the 5 native wolf objects differ (their DWARF producer names the compiler), so the compare fires |
| `reuse-first-real-use.log` | `44387d0` → `2807967` | mz2's pin check at its first real use: the baseline checkout, 0.2.29 still staged, moved to the pin commit. Before the fetch `tools/build` exits 2 on both tiers ("the pin wants 'wolf 0.2.30 (wolfgang, pin 1b37f36)' … found 'wolf 0.2.29 (wolfgang, pin 6ffe7c6)' … not used"); after it, built by 0.2.30 with 0.2.29 still staged beside it (producers `wolfgang 0.2.30`), 9/9 both tiers; `tests/stale-compiler` 5 of 5 on both tiers |
| `machines-and-exit2.log` | `074a349` | the archives' sha256 (the second read of each digest); the checked machine and lupin 0.1.53 refuse by name; a planted E0401 makes `tools/build` exit 2 on both tiers and leave no object; `tools/difftest` after it pass=0 fail=9 exit 1, no line says skip; `tests/stale-compiler` exit 1; the plant restored |

The digests, each read twice: wolf 0.2.30 linux x86-64 `4e55b39e…89`,
79,910,152 bytes, from the release API and from `sha256sum` on kasumi
(and a third time by `tools/fetch-toolchain` in CI); lupin 0.1.53
`5db55332…aa`, 2,945,862 bytes, the same two ways.

The plant (P8), a branch of its own: `mz3-plant` is the pin commit
`2807967` plus `d3782f5`, which changes one character of the wolf digest
(`4e55b39e` → `4e55b39f`). It was seen red in run 38949850825
(workflow_dispatch): both jobs fail at the step `the pinned wolf, by
digest`, "digest 4e55b39ed243… pin wants 4e55b39fd243…", exit 1.

The machines:

- native and release: `wolf build --target x86_64-unknown-none
  --emit=obj`. muzzle's tests run on these two and nowhere else.
- checked (`wolf conform-run --json --checked muzzle.lu`): `verdict:
  unsupported`, "a link-time symbol (`extern "c" let`)", as at 0.2.29.
- lupin 0.1.53 (`lupin run muzzle.lu`): refused by name, the link-time
  symbol `__muzzle_bins` ([abi.link.extern]), exit 4, as 0.1.52.
- `wolf run --checked` is the native build, not the checked machine:
  exit 4, no `main`.

## 5. Score

9 of 9.

- P1, P2: 6 and 4 objects, 19 exports, 0 imports; 9/9 on both tiers,
  each case's exit status and line count equal to 0.2.29's.
- P3: **the pin moves no output.** 12 of 12 stripped objects
  byte-identical; the unstripped control differs in the 5 native wolf
  objects. s755's lowering change did not reach muzzle.
- P4: the pin forced `wolf-toolchain.toml` and README's build line;
  no source, test or header edit.
- P5: refusal exit 2 naming both versions before the fetch; built by
  0.2.30 after it; the test 5 of 5 on both tiers.
- P6: both refuse by name. P7: `?? src/.lu-cache/` at trunk, an empty
  `git status --porcelain` with the line. P8: plant red at the fetch
  step on both jobs (run 38949850825); the head's run is in the PR.
  P9: exit 2 on both tiers, every gate after it non-zero.

## 6. Found, not changed, not filed

- A refused `tools/build` leaves the previous `build/<tier>/` in place
  (the version check runs before `rm -rf "$out"`), so on a reused
  checkout `tools/build native; tools/difftest native` tests the OLD
  compiler's archive and can pass (`reuse-first-real-use.log`: after
  both refusals `build/native` and `build/release` still hold 0.2.29's
  objects). CI is not exposed: a failed build step ends the job.
- `tools/fetch-toolchain` still leaves older directories in
  `.wolf-bin/` (mz2's finding; disk only, since `tools/build` refuses
  them).
