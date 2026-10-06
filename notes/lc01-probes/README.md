# lc01 probes (wolf 0.2.24, kasumi, 2026-10-06)

Throwaway programs that measured what wolf 0.2.24 gives muzzle's first
layer, before any library code. Built on kasumi under ~/lanes/lc01/probes
with the release archive wolf-0.2.24-x86_64-unknown-linux-gnu.tar.gz,
sha256 501d6d3f45124c84…, clang 23.1.1, kernel 7.2.8-1-cachyos.

| log | sha256 | produced by |
|---|---|---|
| probes.log | 0c55b52e9e82… | run.sh. q1 (four spellings of a module var's address); q2 as first written (`q_syscall3(…)` as a unit fn's tail, E0401, both tiers); q3 nm |
| probes2.log | 2e85fbc5ca82… | run3.sh **without** its two later edits (no `-fno-stack-protector`, the q3 disassembly enabled). q2 with `let r = …`: builds, the link fails on `__stack_chk_fail`; q3 disassembly; q4 with its cast outside `unsafe` (E1301) |
| probes3.log | 34e5239f6daf… | run3.sh as committed: q2 links `-nostdlib -static` and runs on both tiers (`q2 hello`, `errno=9`, exit 7) |
| q4.log | 9e1fb34da9b2… | an inline loop over both tiers: `wolf build q4/lib.lu --target x86_64-unknown-none --emit=obj [--release]`, then `clang -o q4 main.c lib.o` (hosted, for printf) and `./q4` |

q2/lib.lu and q4/lib.lu are committed in their final form; the earlier
forms are visible in the logs' own diagnostics (E0401 at the tail call,
E1301 at the cast).
