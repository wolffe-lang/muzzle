# mz4: muzzle at wolf 0.2.32

Lane mz4 (crawl 54, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz4-muzzle-at-0232.md` (planning commit `85886ff`,
authored 2026-10-18). One deliverable: the pin (wolf 0.2.32, lupin
0.1.54) in its own commit, `tools/build`'s pin check passing, build and
tests green on both tiers, every compiled object compared with 0.2.30's
(naming each moved object and why, if s237's shift rule or anything else
moves one); and muzzle#7 (a refused `tools/build` leaves the previous
`build/<tier>/` in place) if small. This file holds the inputs as
re-derived and the prediction, committed before the first change; the
evidence and the score follow.

## 1. Forbidden

Nothing changes beyond the pin, what the pin forces, #7's fix and its
test, and the lane's note, logs and scripts. The licence exception text
is not touched. No C library's source is read. Toolchains come from the
release archives by digest, each digest read twice. No build on
nomad-1; kasumi only, under `~/lanes/mz4/`. No merge, no tag, no issue
filed or closed. No attribution trailers.

## 2. Inputs, verified (2026-10-18)

| input | at origin |
|---|---|
| muzzle trunk | `da22e9e` (pin wolf 0.2.30, `1b37f36`, linux x86-64 `4e55b39e…`); no drift from the contract |
| wolf 0.2.32 | tag `v0.2.32` (tag object `d21c4b28`) → `72de141f624662fdff522e8beba459fab7c5cf66` (wolf-lang); release published 2026-10-18T08:38:14Z, not a draft, not a prerelease; asset `wolf-0.2.32-x86_64-unknown-linux-gnu.tar.gz`, 80,092,071 bytes, release API digest sha256 `4c37f9c5dc3d8830c06810948027d38bdbc375dc5524ab23127b0ada0907d851` (read 1; read 2 is `sha256sum` of the download on kasumi, §4) |
| lupin 0.1.54 | tag `v0.1.54` (tag object `9c98d445`) → `0e536e8fb4518eaa798ac426901ab81275db5c9f` (wolf-interp); release published 2026-10-17T16:48:34Z; asset `lupin-0.1.54-x86_64-unknown-linux-gnu.tar.gz`, 2,971,403 bytes, release API digest sha256 `175df23880cd5bb993b778e5c8fdd7d0d96c176696802e640f6fc87d3dab40dd` (read 1; read 2 on kasumi). 0.2.32's `crates/wolf_driver/PAIRING` names `lupin-version = 0.1.54` |
| wolf 0.2.30 (the comparison) | `4e55b39e…89` as pinned at trunk, read again on kasumi |
| std | muzzle imports no std and links no runtime, so wolf-std (`d37c911`, bundled in 0.2.32) is not an input; nothing to re-derive |
| bump notes read | 0.2.31's and 0.2.32's "read this before you bump" at `v0.2.32:CHANGELOG.md`; muzzle#7 |

What 0.2.31 and 0.2.32 say that can reach muzzle (freestanding, no
std, no runtime linked, no `main`, no host builtin):

- **The shift rule (s237, ruling #77 A)**: a plain-integer shift now
  checks its count and round trip, so "the shift rule changes every
  plain shift's code" and the release tier's inlining can move. muzzle's
  691 lines of wolf hold **no shift operator**: `<<` and `>>` occur once,
  in a comment (`src/heap/heap.lu:13`, "16 << class"). s237's census did
  not include muzzle; this is the lane's own count.
- **Exit 71 at start, the hub and the set (s239, s243, s245)**: entry-shim
  calls in a program that reaches a parking net call or a hub call.
  muzzle has no `main` and no entry shim (`--emit=obj` for
  `x86_64-unknown-none`) and calls no host builtin.
- **The seven new prelude names (0.2.31)**: none declared or used in
  `src/`, `tests/` or `tools/` (grep exit 1; a control grep for `extern`
  in `src/sys/sys.lu` finds 3). 0.2.31's own grep at `da22e9e9` agrees.
- **s245's `live_functions`**: dead-function elimination's reach walk
  made public; read in the diff (`crates/wolf_wir/src/midend/mod.rs`,
  `v0.2.30..v0.2.32`) it is a refactor, the same roots and edges.
- 0.2.32's downstream line: "**muzzle:** the pin."

Carried from mz2 and mz3 and still true: no muzzle file pins lupin and
no muzzle tool runs it, so lupin 0.1.54 is taken by digest only to
record what it answers (P6); no `[lupin]` table is added.

muzzle#7, re-derived from `tools/build` at `da22e9e`: the version check
(lines 21–35) exits 2 before `rm -rf "$out"` (line 38), so a refusal
leaves the previous `build/<tier>/`, and `tools/difftest` reads
`build/<tier>/` without asking which compiler wrote it. **Small**: the
fix moves the tier check and the removal above the version check. The
test goes into `tests/stale-compiler`: every case's tree is seeded with
an earlier build's `build/<tier>/`, and a refusal must leave none.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.32 `tools/build` passes on both tiers: native 6 objects, release 4, 19 exports, 0 imports, as at 0.2.30 | any error or warning, another object count, or the gate fails |
| P2 | `tools/difftest` 9/9 on both tiers; every case's exit status and line count equal to 0.2.30's | any case differs |
| P3 | **the pin moves no output**: all 12 objects (6 native, 4 release, `crt0.o` on each), debug info stripped, byte-identical to 0.2.30's from the same source. No shift, no entry shim, no host call, so neither the shift rule nor the start shims reach muzzle. The compare fires: unstripped, the 5 native wolf objects differ (their DWARF producer names the compiler) | any stripped object differs, or the unstripped control shows no difference |
| P4 | the pin forces two edits and no other: `wolf-toolchain.toml` and README's `wolf 0.2.30` build line | a source, test or header edit is needed for the pin |
| P5 | the pin check on a reused checkout with 0.2.30 still staged: after the pin moves and before `tools/fetch-toolchain`, `tools/build` refuses with exit 2 naming 0.2.30 found and 0.2.32 wanted; after the fetch it builds with 0.2.32 (native producers `wolfgang 0.2.32`). `tests/stale-compiler` 5 of 5 on both tiers | a build with 0.2.30 after the pin moves, or the test below 5 of 5 |
| P6 | the checked machine (`wolf conform-run --json --checked`) and lupin 0.1.54 each refuse muzzle by name (the link-time symbol), as at 0.2.30 / 0.1.53 | either runs it, or dies without naming a reason |
| P7 | #7 at the pin, before the fix: in that reused checkout both refusals leave 0.2.30's `build/native/` and `build/release/` in place, and `tools/difftest native` after the refusal passes 9/9 on the old archive (the bug, shown) | no output left, or difftest fails |
| P8 | #7 after the fix: a refusal exits 2 and leaves no `build/<tier>/`; `tools/difftest <tier>` after it fails all 9 (muzzle column does not build), exit 1; `tests/stale-compiler` with the new assertion 5 of 5 on both tiers; with only `tools/build` reverted to trunk's order, it is 2 of 5 (the three refusal cases fail, "left the previous build"), exit 1 | otherwise |
| P9 | CI: the plant (the head with `tools/build`'s fix reverted, a branch of its own) is red on both jobs at `the stale compiler` step with pass=2 fail=3; the head is green on both jobs | otherwise |
| P10 | no muzzle gate reads `wolf build`'s exit 2 as a skip at 0.2.32: with a compile error planted in `src/` (never committed), `tools/build` exits 2 on both tiers and leaves no object | a zero exit, or a skip |
