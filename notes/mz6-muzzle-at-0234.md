# mz6: muzzle at wolf 0.2.34

Lane mz6 (crawl 54, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz6-muzzle-at-0234.md` (planning commit `280e739`,
authored 2026-10-22), under `sprints/RULES.md`. One deliverable: the
pin (wolf 0.2.34, lupin 0.1.55) in its own commit, with
`wolf-toolchain.toml`'s `version` and `version_line` at
`wolf 0.2.34 (wolfgang, pin 9da7bdc)` and `tools/build`'s pin check
passing; build and tests green on both tiers; every compiled object
compared with 0.2.33's, naming each moved object and why if anything
moves one. Nothing else: muzzle has no open issues. Every new row seen
red before green, planted in CI (RULES.md §7: on muzzle, the PR run
itself). This file holds the inputs as re-derived and the prediction,
committed before the first change; the evidence and the score follow.

## 1. Forbidden

Nothing changes beyond the pin, what the pin forces, the CI plants
(each reverted in the next commit, both kept in the history as their
evidence) and the lane's note, logs and scripts. No source, header,
test, tool or workflow edit. The licence texts are not touched. No C
library's source and no kernel's is read (man pages, POSIX, the C
standard, black-box runs only). Toolchains come from the release
archives by digest, each digest read twice. No build on nomad-1;
kasumi only, under `~/lanes/mz6/`. No merge, no tag, no issue filed or
closed. No attribution trailers.

## 2. Inputs, verified (2026-10-21)

| input | at origin |
|---|---|
| muzzle trunk | `fab3ae9` (mz5 at the 0.2.33 pin `2a81c25`, then tl28's one-tier CI: `ci/tiers.py`, `tier full`, the nightly); no drift from the contract. Pin at trunk: wolf 0.2.33, `85dab5d7`, linux x86-64 `4f955e8d…af8`. Open issues: none; open PRs: none (`gh issue list`, `gh pr list`, both `[]`) |
| wolf 0.2.34 | tag `v0.2.34` (tag object `38ab81cb`) → `9da7bdcf2aea9adf48df6570bedc58e16e9ef853` (wolf-lang); release 419552903, published 2026-10-21T12:58:10Z, not a draft, not a prerelease. Asset `wolf-0.2.34-x86_64-unknown-linux-gnu.tar.gz`, 80,433,878 bytes, release API digest sha256 `88a9dd3da1c9a0e38c08eb6e2de9c1ef04d1ed93dcf51024d8dbbb76890c1821` (read 1); `sha256sum` of the download on kasumi gives the same (read 2). The other three by the same two reads: aarch64 linux `ca5cadf936dc0d4396e92733fc65e953d549f3b61f0aa354e7be2e78ea8c3ad0` (76,212,750 bytes), macOS `29c8331f5d188b00c4580bc7677b1238891c5734d00b0512df10c60eab236ee3` (18,925,363), windows `77bb8403aeeae9f0ebd5a461f3a71d6360acb4be60194307993245d0ff1e2f07` (15,911,654), each the contract's prefix; only the linux x86-64 one is pinned (muzzle builds on x86-64 Linux only). `wolf --version`: `wolf 0.2.34 (wolfgang, pin 9da7bdc)`, `paired with lupin 0.1.55 (reference interpreter), pin 294d626` |
| lupin 0.1.55 | tag `v0.1.55` (tag object `047184fb`) → `e642f2630ad4c7f53740428d5cca705f2e0d295b` (wolf-interp); release 418932753, published 2026-10-20T22:11:27Z. Asset `lupin-0.1.55-x86_64-unknown-linux-gnu.tar.gz`, 2,966,156 bytes, release API digest `1152abc0bef65d334b94bc881a26463fac345919cc2ca07a291796e6048a8a38` (read 1); the same by `sha256sum` on kasumi (read 2). `lupin --version`: `lupin 0.1.55 (wolf-interp, reference interpreter at pin 294d626)`; `294d626` is the wolf-lang conformance pin 0.2.34's `PAIRING` names (`lupin-pin = 294d626`, unchanged from 0.2.33), not drift from `e642f26` |
| wolf 0.2.33 (the comparison) | `4f955e8dfe6eaafa441903eeba43c009c4ec2b45a8da09fe13431e8b12340af8`, 80,128,936 bytes, as pinned at trunk; `sha256sum` on kasumi again |
| std | the 0.2.34 archive's `std/STD-REV` is `e036bec318ad81ecd5a025612bc2c2adb17527f3`, the contract's wolf-std `e036bec`. muzzle imports no std (`grep -rnE '^\s*use\s+std' src` exit 1) and links no runtime (`libwolf_rt.a` is never on its link line), so the std is not an input |
| bump notes read | 0.2.34's "Read this before you bump the pin" and "What each downstream does to bump" at `v0.2.34:CHANGELOG.md`: "**boreutils, muzzle:** the pin." |

What 0.2.34 says that can reach muzzle (freestanding
`x86_64-unknown-none`, `--emit=obj`, `--deny-warnings`, no std, no
runtime linked, no `main`, no host builtin):

- **Seven new prelude names** (`os_fork`, `os_term_restore_at_exit`,
  `os_signal_wait_fds`, `fs_sync`, `fs_sync_dir`, `fs_realpath`,
  `os_clock_ns`) and **one new mark**, `busy`: a function of one of
  these names shadows a builtin, W0304, fatal under `--deny-warnings`,
  which `tools/build` passes. None is declared or used in `src/`,
  `tests/`, `tools/`, `include/` or `ci/` (`grep -rnwE` over the eight
  words, exit 1). Control, that the pattern can match: the same seven
  names over `v0.2.34:crates/wolf_sema/src/prelude.rs` count 7 lines,
  over `v0.2.33`'s 0.
- **A test file's own `test_*` under `wolf test`** (#799): muzzle runs
  no `wolf test` (its tests are `tools/difftest`, C against glibc).
- **The mode int's bits 5–7, the signal hub and wake pipe before
  `main` for `os_signal_wait_fds`, the restore at exit, the fork's
  `busy`**: runtime behaviour and the entry shim. muzzle calls no `os_`
  builtin (`grep -rnE '\bos_[a-z_]+' src` exit 1; control: `extern` in
  `src/sys/sys.lu` counts 3), has no `main` (`grep -rnE '\bfn main\b'
  src` exit 1), so no entry shim is appended, and links no runtime.
- **The compiler diff `v0.2.33..v0.2.34`**, read under `crates/`
  outside tests (25 files): `wolf_codegen_clif/src/lib.rs` adds
  `TERM_RESTORE_EXIT`, `names_a_restore`, a fourth `HUB_SIGNAL` row
  and the `os_signal_wait_fds` arm of `names_a_descriptor_wait`, all
  read only by `add_entry_shim` (main only); `translate.rs` adds eight
  `RT_SYMBOLS` rows (229 → 237), declared on first use only (the
  `self.rt` lookup falls back to the table when a call names the
  symbol); `wolf_wir/src/lower.rs` routes the seven names to their
  builtin lowerings (`fs_realpath` shares `fs_read_text`'s arm by name);
  `wolf_sema` (`check.rs`, `prelude.rs`, `ctfe/intrinsics.rs`) adds
  their signatures, prelude rows and the comptime refusal;
  `wolf_mem/src/ubcheck.rs` is the checked machine's rows and the test
  discovery's file argument; `wolf_rt` is the runtime; `wolf_driver`
  moves `STD-PIN` and `PAIRING` and adds capability tests and the test
  discovery's argument. Every compiler line is behind one of the seven
  names or `main`; `wolf_wir/src/midend` and every other optimiser or
  code-generation path for ordinary code is untouched (`git diff
  --stat v0.2.33 v0.2.34 -- crates/wolf_wir crates/wolf_codegen_clif`:
  `lower.rs`, `lib.rs`, `translate.rs` only).
- The pairing moves to lupin 0.1.55. No muzzle file pins lupin and no
  muzzle tool runs it (as at mz2 to mz5), so lupin is taken by digest
  only to record what it answers (P10); no `[lupin]` table is added.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.34 `tools/build` passes on both tiers: native 6 objects, release 4, 19 exports, 0 imports, as at 0.2.33 | any error or warning, another object count, or the gate fails |
| P2 | `tools/difftest` 9/9 on both tiers, and again under `taskset -c 0-3`; the 18 `PASS` lines (name, exit status, line count) identical to 0.2.33's | any case differs |
| P3 | **the pin moves no output**: all 12 objects (6 native, 4 release, `crt0.o` on each), debug info stripped, byte-identical to 0.2.33's from the same source. The 0.2.33 baseline's 12 stripped hashes equal mz5's committed `pin-0.2.33.log` line for line (the build reproduces across days and trees). The compare fires: unstripped and built in another directory, the 5 native wolf objects differ (`DW_AT_comp_dir` and `DW_AT_producer`); built in the baseline's own directory, each differs in exactly one byte, the producer's last version digit (`wolfgang 0.2.33` → `0.2.34`), and the release objects and both `crt0.o` are identical unstripped | any stripped object differs, a same-path native object differs in more than that byte, or the unstripped control shows no difference |
| P4 | the pin forces two edits and no other: `wolf-toolchain.toml` (version, rev, version line, linux x86-64 digest) and README's `wolf 0.2.33` build line (`git grep '0\.2\.33' -- ':!notes/'` exit 1 after them) | a source, test, tool, header or workflow edit is needed |
| P5 | the pin check on a reused checkout with 0.2.33 still staged and its objects in `build/`: moved to the pin and before `tools/fetch-toolchain`, `tools/build` exits 2 on both tiers naming `wolf 0.2.33 (wolfgang, pin 85dab5d)` found and `wolf 0.2.34 (wolfgang, pin 9da7bdc)` wanted, and leaves no `build/<tier>/`; `tools/difftest` after it pass=0 fail=9, exit 1. After the fetch it builds with 0.2.34 (native producers `wolfgang 0.2.34`), 9/9; `tests/stale-compiler` 5 of 5 on both tiers | a build with 0.2.33 after the pin moves, output left by a refusal, or the test below 5 of 5 |
| P6 | the digest check refuses: with the pin's linux x86-64 digest changed in its last hex digit, `tools/fetch-toolchain` exits 1 with `fetch-toolchain: wolf-0.2.34-x86_64-unknown-linux-gnu.tar.gz digest 88a9dd3d…821, pin wants <the planted digest>`, on both paths (`WOLF_ARCHIVE` and the release download), and extracts no driver; `tools/build` after it exits 2 (no driver at the pin's path). The 0.2.33 archive offered for the 0.2.34 pin is refused the same way, naming `4f955e8d…af8` | a zero exit, a driver staged, or a refusal that does not name both digests |
| P7 | the version-line check refuses: with the real digest and the pin's `version_line` changed in the pin sha's last digit (`pin 9da7bdc` → `pin 9da7bdd`), `tools/fetch-toolchain` exits 1 with `fetch-toolchain: 'wolf 0.2.34 (wolfgang, pin 9da7bdc)' is not the pinned 'wolf 0.2.34 (wolfgang, pin 9da7bdd)'`. It has extracted the driver by then (the check runs the staged `wolf --version`), so `tools/build` after it finds a driver at the pin's path and refuses it at its own pin check: exit 2 on both tiers, `found 'wolf 0.2.34 (wolfgang, pin 9da7bdc)' … the pin wants 'wolf 0.2.34 (wolfgang, pin 9da7bdd)'`, no `build/<tier>/` left | a zero exit at either tool, or a refusal not naming both lines |
| P8 | CI, the plants, in the PR run itself (RULES.md §7), each pushed alone at the head and reverted by the next commit: (a) P6's digest digit: both `difftest` jobs red at the step `the pinned wolf, by digest`, exit 1, the refusal line naming `88a9dd3d…821` and the planted digest; `the stale compiler`, `build` and `difftest` skipped, `keep the outputs` run; `tier full` red, its verdict naming `difftest failure`. (b) P7's version-line digit: the same jobs red at the same step with the version-line refusal; `tier full` red | a green job, or a red at any other step |
| P9 | no muzzle gate reads `wolf build`'s exit 2 as a skip at 0.2.34: with a compile error planted in `src/` (never committed), `tools/build` exits 2 on both tiers and leaves no object; `tools/difftest` after it pass=0 fail=9 exit 1, no skip line; `tests/stale-compiler` exit 1 | a zero exit, or a skip |
| P10 | the checked machine (`wolf conform-run --json --checked`) and lupin 0.1.55 each refuse muzzle by name (the link-time symbol), as at 0.2.33 | either runs it, or dies without naming a reason |
| P11 | CI at the head: `difftest (native)`, `difftest (release)` and `tier full` green (stale-compiler 5/5, build, difftest 9/9 per tier) | otherwise |
