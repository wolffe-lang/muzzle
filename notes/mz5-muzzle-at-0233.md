# mz5: muzzle at wolf 0.2.33

Lane mz5 (crawl 54, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz5-muzzle-at-0233.md` (planning commit `73e7c1d`,
authored 2026-10-19). One deliverable: the pin (wolf 0.2.33, lupin
0.1.54) in its own commit, `tools/build`'s pin check passing, build and
tests green on both tiers, every compiled object compared with 0.2.32's
(naming each moved object and why, if anything moves one). Nothing
else: muzzle has no open issues. The brief adds: the pin check seen
refusing a planted wrong digest in CI. This file holds the inputs as
re-derived and the prediction, committed before the first change; the
evidence and the score follow.

## 1. Forbidden

Nothing changes beyond the pin, what the pin forces, and the lane's
note, logs and scripts. No source, header, test, tool or workflow
edit. The licence exception text is not touched. No C library's source
and no kernel's is read (man pages, POSIX, the C standard, black-box
runs only). Toolchains come from the release archives by digest, each
digest read twice. No build on nomad-1; kasumi only, under
`~/lanes/mz5/`. No merge, no tag, no issue filed or closed. No
attribution trailers.

## 2. Inputs, verified (2026-10-19)

| input | at origin |
|---|---|
| muzzle trunk | `158acbe` (mz4; pin wolf 0.2.32, `72de141`, linux x86-64 `4c37f9c5…51`); no drift from the contract. Open issues: none; open PRs: none (`gh issue list`, `gh pr list`, both `[]`) |
| wolf 0.2.33 | tag `v0.2.33` (tag object `56110e4f`) → `85dab5d7e597680d6fd7034f1d7ed2acb9a92f97` (wolf-lang); release 417610226, published 2026-10-19T18:37:53Z, not a draft, not a prerelease; asset `wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz`, 80,128,936 bytes, release API digest sha256 `4f955e8dfe6eaafa441903eeba43c009c4ec2b45a8da09fe13431e8b12340af8` (read 1); `sha256sum` of the download on kasumi gives the same (read 2). `wolf --version`: `wolf 0.2.33 (wolfgang, pin 85dab5d)`, `paired with lupin 0.1.54 (reference interpreter), pin 294d626` |
| lupin 0.1.54 | tag `v0.1.54` (tag object `9c98d445`) → `0e536e8fb4518eaa798ac426901ab81275db5c9f` (wolf-interp); release 415937354, published 2026-10-17T16:48:34Z; asset `lupin-0.1.54-x86_64-unknown-linux-gnu.tar.gz`, 2,971,403 bytes, release API digest `175df23880cd5bb993b778e5c8fdd7d0d96c176696802e640f6fc87d3dab40dd` (read 1); the same by `sha256sum` on kasumi (read 2). `lupin --version`: `lupin 0.1.54 (wolf-interp, reference interpreter at pin 294d626)`; `294d626` is a wolf-lang commit, the conformance pin lupin declares, as 0.2.33's `PAIRING` says (not a drift from `0e536e8`) |
| wolf 0.2.32 (the comparison) | `4c37f9c5dc3d8830c06810948027d38bdbc375dc5524ab23127b0ada0907d851`, 80,092,071 bytes, as pinned at trunk; `sha256sum` on kasumi again |
| std | the 0.2.33 archive's `std/STD-REV` is `cceda63151430eac07df895fdfffddfa4a0b92c1`, the contract's wolf-std `cceda63`. muzzle imports no std (`grep -rnE '^\s*use\s+std' src` exit 1) and links no runtime (`libwolf_rt.a` and `libwolf_rt_none.a` are never on its link line), so the std is not an input and there is nothing to re-derive against the new compiler (wave 47's B151 rule) |
| bump notes read | 0.2.33's "Read this before you bump the pin" and its downstream list at `v0.2.33:CHANGELOG.md`: "**lobo, muzzle, pax:** the pin." |

What 0.2.33 says that can reach muzzle (freestanding
`x86_64-unknown-none`, `--emit=obj`, no std, no runtime linked, no
`main`, no host builtin):

- **Five new prelude names** (`os_signal_ignore_one`,
  `os_signal_default_one`, `os_exit_signal`, `os_wait_status_core`,
  `os_wait_poll_core`): a function of one of these names shadows a
  builtin, W0304, fatal under `--deny-warnings`, which `tools/build`
  passes. None is declared or used in `src/`, `tests/`, `tools/` or
  `include/` (grep exit 1). Control, that the pattern can match: the
  same grep over `v0.2.33:crates/wolf_sema/src/prelude.rs` counts 5
  lines, over `v0.2.32`'s 0.
- **`os_signal_text(0)`, a sent SEGV or BUS, the descriptor waits'
  wake pipe**: runtime behaviour and an entry-shim start call. muzzle
  calls no `os_` builtin (`grep -rnE '\bos_[a-z_]+' src` exit 1;
  control: `extern` in `src/sys/sys.lu` counts 3), has no `main`
  (`grep -rnE '\bfn main\b' src` exit 1), so no entry shim is appended,
  and links no runtime.
- **The compiler diff `v0.2.32..v0.2.33`**, read under `crates/`
  outside tests: `wolf_codegen_clif` adds `WAIT_FD_START` to
  `add_entry_shim` (main only) and five `RT_SYMBOLS` rows (223 → 229,
  declared only when named); `wolf_wir/src/lower.rs` routes the five
  names to `lower_job_builtin`; `wolf_sema` (`check.rs`, `prelude.rs`,
  `ctfe/intrinsics.rs`) adds their signatures, prelude rows and the
  comptime refusal; `wolf_mem/src/ubcheck.rs` is the checked machine's
  rows for them; `wolf_rt` is the runtime; `wolf_driver` moves
  `STD-PIN` and adds a capability test. Every compiler line is behind
  one of the five names or `main`; no midend, optimiser or code
  generation of ordinary code changed (`wolf_wir/src/midend` untouched).
- The pairing stays lupin 0.1.54. No muzzle file pins lupin and no
  muzzle tool runs it (as at mz2 to mz4), so lupin is taken by digest
  only to record what it answers (P8); no `[lupin]` table is added.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.33 `tools/build` passes on both tiers: native 6 objects, release 4, 19 exports, 0 imports, as at 0.2.32 | any error or warning, another object count, or the gate fails |
| P2 | `tools/difftest` 9/9 on both tiers, and again under `taskset -c 0-3`; the 18 `PASS` lines (name, exit status, line count) identical to 0.2.32's | any case differs |
| P3 | **the pin moves no output**: all 12 objects (6 native, 4 release, `crt0.o` on each), debug info stripped, byte-identical to 0.2.32's from the same source. Nothing in 0.2.33's compiler reaches code without the five names or a `main`. The compare fires: unstripped, the 5 native wolf objects differ (their DWARF producer names the compiler) | any stripped object differs, or the unstripped control shows no difference |
| P4 | the pin forces two edits and no other: `wolf-toolchain.toml` (version, rev, version line, linux x86-64 digest) and README's `wolf 0.2.32` build line | a source, test, tool, header or workflow edit is needed |
| P5 | the pin check on a reused checkout with 0.2.32 still staged: moved to the pin and before `tools/fetch-toolchain`, `tools/build` exits 2 on both tiers naming `wolf 0.2.32 (wolfgang, pin 72de141)` found and `wolf 0.2.33 (wolfgang, pin 85dab5d)` wanted, and leaves no `build/<tier>/` (mz4's #7 fix); `tools/difftest` after it pass=0 fail=9, exit 1. After the fetch it builds with 0.2.33 (native producers `wolfgang 0.2.33`), 9/9; `tests/stale-compiler` 5 of 5 on both tiers | a build with 0.2.32 after the pin moves, output left by a refusal, or the test below 5 of 5 |
| P6 | the digest check refuses: with the pin's linux x86-64 digest changed in its last hex digit, `tools/fetch-toolchain` exits 1 with `fetch-toolchain: wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz digest 4f955e8d…af8, pin wants <the planted digest>` and extracts no driver (`.wolf-bin/wolf-0.2.33-x86_64-unknown-linux-gnu/` absent); `tools/build` after it exits 2 (no driver at the pin's path). The 0.2.32 archive offered for the 0.2.33 pin (`WOLF_ARCHIVE`) is refused the same way, naming `4c37f9c5…51` | a zero exit, a driver staged, or a refusal that does not name both digests |
| P7 | CI, the plant (the pin with that one digit changed, on a branch of its own, `mz5-plant`): both jobs red at the step `the pinned wolf, by digest`, exit 1, the refusal line naming `4f955e8d…af8` and the planted digest; every later step skipped but `keep the outputs` | a green job, or a red at any other step |
| P8 | the checked machine (`wolf conform-run --json --checked`) and lupin 0.1.54 each refuse muzzle by name (the link-time symbol), as at 0.2.32 | either runs it, or dies without naming a reason |
| P9 | no muzzle gate reads `wolf build`'s exit 2 as a skip at 0.2.33: with a compile error planted in `src/` (never committed), `tools/build` exits 2 on both tiers and leaves no object; `tools/difftest` after it pass=0 fail=9 exit 1, no skip line; `tests/stale-compiler` exit 1 | a zero exit, or a skip |
| P10 | CI at the head: both jobs green (stale-compiler 5/5, build, difftest 9/9 per tier) | otherwise |
