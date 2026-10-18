#!/usr/bin/env bash
# mz4: muzzle#7 after the fix. The reused checkout builds at trunk
# (0.2.30, its objects left in build/), moves to the fix commit $1 with
# only 0.2.30 staged, and is refused; then the fix reverted alone, so
# the new assertions are seen to fail against trunk's tools/build.
#   fix7.sh <fix-commit> <pre-fix-commit>
set -uo pipefail
co=$HOME/lanes/mz4/base; arc=$HOME/lanes/mz4/dl/wolf-0.2.32-x86_64-unknown-linux-gnu.tar.gz
cd "$co" || exit 2
git fetch -q origin mz4 || exit 2
git checkout -q da22e9e || exit 2
rm -rf .wolf-bin/wolf-0.2.32-x86_64-unknown-linux-gnu .wolf-bin/wolf-0.2.32-x86_64-unknown-linux-gnu.tar.gz
echo "== at $(git rev-parse HEAD) (trunk, the 0.2.30 pin), staged: $(ls .wolf-bin | tr '\n' ' ')"
for t in native release; do tools/build $t >/dev/null; echo "build $t (0.2.30) exit=$?"; done
ls build/native build/release | tr '\n' ' '; echo
git checkout -q "$1" || exit 2; echo "== now at $(git rev-parse HEAD): $(grep '^version_line' wolf-toolchain.toml)"
for t in native release; do tools/build $t; echo "build $t (0.2.32 pinned, not fetched) exit=$?"; done
echo "== muzzle#7: what the refusals left"
ls -d build/native build/release 2>&1; echo "ls build exit=$?"
echo "== tools/difftest after the refusal"
for t in native release; do tools/difftest $t >"$HOME/lanes/mz4/logs/fix7-difftest-$t.out" 2>&1; echo "difftest $t after the refusal exit=$?"; grep -c '^FAIL .*muzzle column does not build' "$HOME/lanes/mz4/logs/fix7-difftest-$t.out"; tail -1 "$HOME/lanes/mz4/logs/fix7-difftest-$t.out"; done
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
  taskset -c 0-3 tools/difftest $t >"$HOME/lanes/mz4/logs/fix7-taskset-$t.out" 2>&1; echo "difftest $t under taskset -c 0-3 exit=$?"; tail -1 "$HOME/lanes/mz4/logs/fix7-taskset-$t.out"
  tests/stale-compiler $t; echo "stale-compiler $t exit=$?"
done
echo "== the fix reverted alone: tools/build from $2, the test from $1"
git show "$2":tools/build > tools/build
git diff --stat
for t in native release; do tests/stale-compiler $t; echo "stale-compiler $t (fix reverted) exit=$?"; done
git checkout -q tools/build; git status --porcelain; echo "restored, git status exit=$?"
