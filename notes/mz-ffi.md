# mz: muzzle's capabilities line

Lane mz (wave 53, Opus). Contract: wolffe-lang/wolf
`sprints/muzzle/mz-ffi.md` (authored 2026-10-12). One deliverable:
muzzle on wolf 0.2.28 with `capabilities: [ffi]` in `src/wolf.pkg`, its
build and tests green, nothing else changed unless the pin move forces
it. This file holds the inputs as re-derived and the prediction,
committed before the first change; the evidence and the score follow.

## 1. Why

wolf 0.2.27 (s221, wolf-lang#619, ruling #55) charges the `ffi`
capability at every bodyless `extern "c" fn` and every `extern "c" let`,
called or not. muzzle declares four (`src/sys/sys.lu`: `fn
__muzzle_syscall6`, `let __muzzle_errno`, `let __muzzle_scratch`;
`src/heap/heap.lu`: `let __muzzle_bins`), and `src/wolf.pkg` declares no
capabilities, so from 0.2.27 on muzzle's build is E1504 on native and
release. 0.2.27's and 0.2.28's "read this before you bump" sections both
name this one line.

## 2. Inputs, verified (2026-10-12)

| input | at origin |
|---|---|
| muzzle trunk | `7c95300` (pin wolf 0.2.24, `294d626d`, linux x86-64 `501d6d3f…`) |
| wolf 0.2.28 | tag `v0.2.28` → `f71ece8e1ad2…`; asset `wolf-0.2.28-x86_64-unknown-linux-gnu.tar.gz` sha256 `3215f3219c5c6c3b84f0f9abf421cdaecec5b81cf42d92b6c64475f95ad199aa` (GitHub's digest; re-hashed by `tools/fetch-toolchain` on kasumi and in CI) |
| bump notes read | 0.2.25, 0.2.26, 0.2.27, 0.2.28 ("read this before you bump", at `v0.2.28:CHANGELOG.md`) |

What the four bumps say that can reach muzzle (freestanding, no std, no
region, no `str`, no host builtin):

- 0.2.25 (s214, #598/#601/#600): every call now clobbers module `var`s,
  `extern "c" let` storage and raw-pointer memory; "downstream binaries
  gain reloads after calls". muzzle's heap folded its state access to
  dodge #598 (lc01, `43f33c8`), and that code stays as it is.
- 0.2.25, 0.2.26, 0.2.27, 0.2.28: new prelude names (W0304 under
  `--deny-warnings`); each release's grep found no declaration in muzzle
  `7c95300`.
- 0.2.26 (s217): capabilities derived from host builtins; muzzle calls
  none.
- 0.2.27 (s221): the `ffi` line. "With the line their objects are the
  same as 0.2.26 built them." 0.2.27 adds two symbols to
  `libwolf_rt_none.a`, which muzzle never links.
- 0.2.27/0.2.28 E1010 refusals (#618, #660, #665): regions, `str`,
  channels and procs; muzzle has none of them.

## 3. Prediction, committed before the first change

| # | predicted | falsified if |
|---|---|---|
| P1 | at 0.2.28 **without** the line, `tools/build` fails with E1504 on both tiers, naming `ffi` and an `extern "c"` declaration in `src/sys/sys.lu` (the first one, `__muzzle_syscall6` at line 9) | it builds, or the refusal is anything but E1504 |
| P2 | with the line, `tools/build` passes on both tiers: 6 objects, 19 exports, 0 imports, as lc01's `gate-green.log` at 0.2.24 | any other build error or warning, or the gate fails |
| P3 | `tools/difftest` 9/9 on both tiers, every case's stdout and exit status equal to glibc's, as at 0.2.24 | any case differs |
| P4 | the pin move forces exactly one other edit: README's `wolf 0.2.24` in the build instructions. The LEDGER's #598 row, the heap comment and `src/muzzle.lu`'s 0.2.24 mention are history, left as written | a source edit is needed beyond `src/wolf.pkg` |
| P5 | the 0.2.28 objects (debug info stripped) differ from 0.2.24's (s214's reloads after calls through raw pointers and `extern "c" let`) | byte-identical |
| P6 | the 0.2.28 objects with the line equal 0.2.26's without it, after stripping debug info (0.2.27's note) | they differ |
| P7 | CI: the pin-only commit is red on both jobs at the build step (E1504); the head with the line is green on both | otherwise |
