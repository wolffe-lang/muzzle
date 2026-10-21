#!/usr/bin/env bash
# mz6: the pin check on a reused checkout. A checkout that built at
# 0.2.33 (its .wolf-bin/ still holding 0.2.33, its build/<tier>/ holding
# 0.2.33's objects) moves to the 0.2.34 pin commit given as $1.
#   reuse.sh <commit>
set -uo pipefail
L=$HOME/lanes/mz6
co=$L/base; arc=$L/dl/wolf-0.2.34-x86_64-unknown-linux-gnu.tar.gz
cd "$co" || exit 2
git fetch -q origin mz6 || exit 2
prod() {
  for o in build/$1/muzzle.*.o; do
    p=$(readelf --debug-dump=info "$o" 2>/dev/null | grep -m1 DW_AT_producer | sed 's/.*: //')
    echo "producer $1/$(basename "$o"): ${p:-<none>}"
  done
}
echo "== before: $(git rev-parse HEAD), staged: $(ls .wolf-bin | tr '\n' ' ') date=$(date -u +%FT%TZ)"
for t in native release; do tools/build $t >/dev/null; echo "build $t (0.2.33, trunk) exit=$?"; done
ls build/native build/release | tr '\n' ' '; echo
prod native
git checkout -q "$1" || exit 2; echo "== now at $(git rev-parse HEAD): $(grep '^version_line' wolf-toolchain.toml)"
for t in native release; do tools/build $t; echo "build $t (0.2.34 pinned, not fetched) exit=$?"; done
echo "== what the refusals left"
ls -d build/native build/release 2>&1; echo "ls build exit=$?"
echo "== tools/difftest after the refusal"
for t in native release; do tools/difftest $t >"$L/logs/reuse-difftest-$t.out" 2>&1; echo "difftest $t after the refusal exit=$?"; grep -c '^FAIL .*muzzle column does not build' "$L/logs/reuse-difftest-$t.out"; tail -1 "$L/logs/reuse-difftest-$t.out"; done
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
echo "== staged now: $(ls .wolf-bin | tr '\n' ' ')"
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
done
prod native
for t in native release; do tests/stale-compiler $t; echo "stale-compiler $t exit=$?"; done
git status --porcelain; echo "git status exit=$?"
