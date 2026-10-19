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

## 4. Evidence (kasumi, linux x86-64, clang 23.1.1, glibc 2.44)

Each log is the raw output of the script beside it in
`notes/mz5-evidence/scripts/`, every exit status echoed bare (no pipe on
a command under test). `run.sh` starts each tree with `git clean -xfd`
(no `.wolf-bin/`, `build/`, `out/` or `src/.lu-cache/`) and prints the
shell's `SigBlk`/`SigIgn` (both 0 in every run).

| log | commit | what |
|---|---|---|
| `baseline-0.2.32.log` | `158acbe` (trunk), tree `~/lanes/mz5/base` | wolf 0.2.32: build 6 and 4 objects, 19 exports, 0 imports; difftest 9/9 on both tiers, and 9/9 under `taskset -c 0-3`; producers `wolfgang 0.2.32`; its 12 stripped hashes equal mz4's `pin-0.2.32.log` line for line (diff exit 0: the build reproduces across days and trees); `git status` empty after |
| `pin-0.2.33.log` | `b580fc3` (the pin), tree `~/lanes/mz5/muzzle` | wolf 0.2.33: the same counts, 9/9 on both tiers and under `taskset -c 0-3`; the 18 `PASS` lines identical to the baseline's (diff exit 0); producers `wolfgang 0.2.33`; `git status` empty after |
| `objects-0.2.32-0.2.33.log` | both | 12 objects stripped (`objcopy --strip-debug`): 12 same, 0 differ. Control: unstripped, the 5 native wolf objects differ |
| `sections-0.2.32-0.2.33.log` | both | where those 5 differ, by section (`readelf -x` by index, self-check ok): `.debug_info` and `.rela.debug_info`, 29 to 2,738 bytes each, because the two trees have different paths: `DW_AT_comp_dir` is `…/mz5/base/src` against `…/mz5/muzzle/src` (2 bytes longer, so every later debug offset moves), besides `DW_AT_producer` |
| `pin-0.2.33-samepath.log`, `objects-0.2.32-0.2.33-samepath.log`, `sections-0.2.32-0.2.33-samepath.log` | `b580fc3`, tree `~/lanes/mz5/base` (the baseline's own path) | the control that attributes the raw difference: 0.2.33 built in the 0.2.32 baseline's directory; 9/9 both tiers; 12 stripped same; unstripped, each of the 5 native wolf objects differs in **one byte**, in `.debug_info`, the producer's last digit (`cmp -l`: `062` → `063`, `2` → `3`); every other section and byte equal, section header tables equal; the release objects and both `crt0.o` identical unstripped |
| `reuse.log` | `158acbe` → `b580fc3`, tree `~/lanes/mz5/base` | the reused checkout, 0.2.32 staged and its objects in `build/`, moved to the pin: `tools/build` exits 2 on both tiers ("the pin wants 'wolf 0.2.33 (wolfgang, pin 85dab5d)' … found 'wolf 0.2.32 (wolfgang, pin 72de141)' … not used"); no `build/native` or `build/release` left (`ls` exit 2); `tools/difftest` after it pass=0 fail=9, exit 1, on both tiers. After the fetch: producers `wolfgang 0.2.33`, 9/9 both tiers, `tests/stale-compiler` 5 of 5 both tiers |
| `digest.log` | `b580fc3`, tree `~/lanes/mz5/digest` | the digest check: the pin's digest with its last digit 8 → 9 (working tree only, restored): `tools/fetch-toolchain` exit 1, "digest 4f955e8d…af8, pin wants 4f955e8d…af9", with the real archive offered by `WOLF_ARCHIVE` and again downloaded from the release (CI's path); no driver directory extracted (`ls` exit 2); `tools/build native` after it exit 2 ("no driver"). The real pin offered the 0.2.32 archive: exit 1 naming `4c37f9c5…51`, no driver. Control: the real pin and archive stage the driver, exit 0 |
| `machines-and-exit2.log` | `b580fc3` | the three archives' sha256 (the second read of each digest); `std/STD-REV` `cceda63…`; the checked machine and lupin 0.1.54 refuse by name; a planted E0401 (never committed) makes `tools/build` exit 2 on both tiers with `build/<tier>/` empty; `tools/difftest` after it pass=0 fail=9 exit 1, no line says skip (`grep -ci skip` 0); `tests/stale-compiler` pass=3 fail=2 exit 1; the plant restored |
| `plant-run-39310874411.log` | `14c2a4a` | the CI plant's job and step conclusions and its failing lines, from `gh run view --log-failed` |

**The objects, one by one** (stripped sizes; "same path" is the
`samepath` control, "other path" the pin run in its own tree):

| tier | object | stripped, 0.2.32 → 0.2.33 | unstripped, same path | unstripped, other path |
|---|---|---|---|---|
| native | `crt0.o` (776 B) | identical | identical | identical |
| native | `muzzle.asm-sys.o` (784 B) | identical | identical | identical |
| native | `muzzle.heap.o` (8,184 B) | identical | 1 byte: producer digit (@5144) | 706 bytes, `.debug_info` + `.rela.debug_info` |
| native | `muzzle.process.o` (1,248 B) | identical | 1 byte: producer digit (@569) | 186 bytes, the same two sections |
| native | `muzzle.root.o` (1,088 B) | identical | 1 byte: producer digit (@327) | 29 bytes, the same two sections |
| native | `muzzle.string.o` (5,512 B) | identical | 1 byte: producer digit (@3799) | 601 bytes, the same two sections |
| native | `muzzle.sys.o` (5,432 B) | identical | 1 byte: producer digit (@2987) | 2,738 bytes, the same two sections |
| release | `crt0.o` (776 B) | identical | identical | identical |
| release | `muzzle.asm-sys.o` (784 B) | identical | identical | identical |
| release | `muzzle.c00.o` (5,384 B) | identical | identical | identical |
| release | `muzzle.c01.o` (2,280 B) | identical | identical | identical |
| release | `muzzle.c02.o` (2,544 B) | identical | identical | identical |

**The pin moved no output.** No code, data, symbol or relocation byte
moved on either tier; the only byte 0.2.33 changes in any object is
the version digit in the native wolf objects' `DW_AT_producer`.

The digests, each read twice: wolf 0.2.33 linux x86-64 `4f955e8d…af8`,
80,128,936 bytes, from the release API and from `sha256sum` on kasumi
(and asserted equal to the pin's line, non-empty, before the pin
commit; a third time by `tools/fetch-toolchain` in CI); lupin 0.1.54
`175df238…dd`, 2,971,403 bytes, the same two ways; wolf 0.2.32
`4c37f9c5…51` again on kasumi for the baseline.

The plant (P7), a branch of its own: `mz5-plant` is the pin `b580fc3`
plus `14c2a4a`, which changes the pin's linux x86-64 digest's last
digit (`…af8` → `…af9`) and nothing else. It was seen red in run
39310874411 (workflow_dispatch, head `14c2a4a`): both jobs fail at the
step `the pinned wolf, by digest`, "fetch-toolchain:
wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz digest 4f955e8d…af8, pin
wants 4f955e8d…af9", exit code 1; `the stale compiler`, `build` and
`difftest` skipped, `keep the outputs` run
(`notes/mz5-evidence/plant-run-39310874411.log`).

The machines:

- native and release: `wolf build --target x86_64-unknown-none
  --emit=obj`. muzzle's tests run on these two and nowhere else.
- checked (`wolf conform-run --json --checked muzzle.lu`): `verdict:
  unsupported`, "a link-time symbol (`extern "c" let`)",
  `phase_reached: mem`, as at 0.2.32.
- lupin 0.1.54 (`lupin run muzzle.lu`): refused by name, the link-time
  symbol `__muzzle_bins` ([abi.link.extern]), exit 4, as at 0.2.32.
- `wolf run --checked` is the native build, not the checked machine:
  exit 4, no `main`.

## 5. Score

10 of 10 (P10 is the head's run, cited in the PR).

- P1, P2: 6 and 4 objects, 19 exports, 0 imports; 9/9 on both tiers
  and under `taskset -c 0-3`; the 18 `PASS` lines identical to 0.2.32's.
- P3: **the pin moves no output.** 12 of 12 stripped objects
  byte-identical to 0.2.32's; the unstripped control differs in the 5
  native wolf objects only, and from the same path in one byte each,
  the producer's version digit.
- P4: the pin forced `wolf-toolchain.toml` and README's build line;
  no other file (`git grep '0\.2\.32' -- ':!notes/'` exit 1 after them).
- P5: refusal exit 2 naming both versions before the fetch, no
  `build/<tier>/` left, difftest 0/9; built by 0.2.33 after it; the
  test 5 of 5 on both tiers.
- P6: the planted digest refused on both paths and the 0.2.32 archive
  refused, each exit 1 naming both digests, no driver staged.
- P7: the plant red at `the pinned wolf, by digest` on both jobs, run
  39310874411, with the predicted line.
- P8: both refuse by name.
- P9: exit 2 on both tiers, every gate after it non-zero, no skip.

## 6. Found, not changed, not filed

- **mz4's raw-object attribution was incomplete.** Its note says the 5
  native wolf objects differ unstripped by "their DWARF producer"; its
  0.2.30 tree was `~/lanes/mz4/base` and its 0.2.32 tree
  `~/lanes/mz4/muzzle`, so `DW_AT_comp_dir` differed too
  (`sections-0.2.32-0.2.33.log` shows the same shape here). Its
  conclusion, stripped objects identical, stands; the same-path control
  above is what isolates the producer.
- `tools/fetch-toolchain` keeps a refused download in `.wolf-bin/`
  under the pinned archive's name (`digest.log`: after the 0.2.32
  archive is refused, `.wolf-bin/wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz`
  holds 0.2.32's bytes). It is never extracted and the next fetch
  overwrites it, so nothing builds from it; disk and confusion only.
- `tools/fetch-toolchain` still leaves older directories in
  `.wolf-bin/` (mz2's finding, `reuse.log`: 0.2.32 and 0.2.33 both
  staged; `tools/build` refuses the old one).
- `tools/build` makes `build/<tier>/` before `wolf build` runs, so a
  compile error leaves it empty rather than absent (`machines-and-exit2.log`);
  difftest fails all 9 either way.
