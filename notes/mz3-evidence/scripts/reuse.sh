#!/usr/bin/env bash
# mz3: mz2's pin check at its first real use. A checkout that built at
# 0.2.29 (its .wolf-bin/ still holding 0.2.29) moves to the 0.2.30 pin.
set -uo pipefail
co=$HOME/lanes/mz3/base; arc=$HOME/lanes/mz3/dl/wolf-0.2.30-x86_64-unknown-linux-gnu.tar.gz
cd "$co" || exit 2
git remote set-url origin https://github.com/wolffe-lang/muzzle.git; git fetch -q origin mz3 || exit 2
echo "== before: $(git rev-parse HEAD), staged: $(ls .wolf-bin | tr '\n' ' ')"
git checkout -q "$1" || exit 2; echo "== now at $(git rev-parse HEAD): $(grep '^version_line' wolf-toolchain.toml)"
for t in native release; do tools/build $t; echo "build $t (0.2.30 pinned, not fetched) exit=$?"; done
ls build/native build/release 2>&1; echo "ls build exit=$?"
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
echo "== staged now: $(ls .wolf-bin | tr '\n' ' ')"
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
done
for o in build/native/muzzle.*.o; do
  p=$(readelf --debug-dump=info "$o" 2>/dev/null | grep -m1 DW_AT_producer | sed 's/.*: //')
  echo "producer $(basename "$o"): ${p:-<none>}"
done
for t in native release; do tests/stale-compiler $t; echo "stale-compiler $t exit=$?"; done
