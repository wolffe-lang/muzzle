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
