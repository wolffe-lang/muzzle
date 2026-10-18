#!/usr/bin/env bash
# mz4: the pin check on a reused checkout, and muzzle#7 shown. A
# checkout that built at 0.2.30 (its .wolf-bin/ still holding 0.2.30)
# moves to the 0.2.32 pin commit given as $1.
#   reuse.sh <commit>
set -uo pipefail
co=$HOME/lanes/mz4/base; arc=$HOME/lanes/mz4/dl/wolf-0.2.32-x86_64-unknown-linux-gnu.tar.gz
cd "$co" || exit 2
git fetch -q origin mz4 || exit 2
echo "== before: $(git rev-parse HEAD), staged: $(ls .wolf-bin | tr '\n' ' ')"
git checkout -q "$1" || exit 2; echo "== now at $(git rev-parse HEAD): $(grep '^version_line' wolf-toolchain.toml)"
prod() {
  for o in build/$1/muzzle.*.o; do
    p=$(readelf --debug-dump=info "$o" 2>/dev/null | grep -m1 DW_AT_producer | sed 's/.*: //')
    echo "producer $1/$(basename "$o"): ${p:-<none>}"
  done
}
for t in native release; do tools/build $t; echo "build $t (0.2.32 pinned, not fetched) exit=$?"; done
echo "== muzzle#7: what the refusals left"
ls build/native build/release 2>&1; echo "ls build exit=$?"
prod native
echo "== muzzle#7: tools/difftest after the refusal (reads build/<tier>/, whatever wrote it)"
tools/difftest native; echo "difftest native after the refusal exit=$?"
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
echo "== staged now: $(ls .wolf-bin | tr '\n' ' ')"
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
done
prod native
for t in native release; do tests/stale-compiler $t; echo "stale-compiler $t exit=$?"; done
git status --porcelain; echo "git status exit=$?"
