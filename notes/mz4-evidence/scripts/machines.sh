#!/usr/bin/env bash
# mz4: what the checked machine and lupin 0.1.54 answer for muzzle, and
# exit 2 at 0.2.32 (a compile error planted, never committed).
set -uo pipefail
X=$HOME/lanes/mz4/x; W=$X/wolf-0.2.32-x86_64-unknown-linux-gnu/wolf; L=$X/lupin-0.1.54-x86_64-unknown-linux-gnu/lupin
cd $HOME/lanes/mz4/muzzle/src || exit 2
echo "== commit $(git rev-parse HEAD) date=$(date -u +%FT%TZ)"
sha256sum $HOME/lanes/mz4/dl/*.tar.gz
$W --version; $L --version
echo; echo "#### the checked machine: wolf conform-run --json --checked"
echo "\$ $W conform-run --json --checked muzzle.lu"; $W conform-run --json --checked muzzle.lu; echo "exit=$?"
echo; echo "#### wolf run --checked (the NATIVE build, not the checked machine)"
echo "\$ $W run --checked muzzle.lu"; $W run --checked muzzle.lu; echo "exit=$?"
echo; echo "#### lupin 0.1.54"
echo "\$ $L run muzzle.lu"; $L run muzzle.lu; echo "exit=$?"
echo "\$ $L conform-run muzzle.lu"; $L conform-run muzzle.lu; echo "exit=$?"
echo; echo "#### exit 2: a type error planted in src/string/string.lu (restored after)"
cd ..
printf '\nfn mz4_planted() -> i32 {\n    return "not an i32"\n}\n' >> src/string/string.lu
git diff --stat
for t in native release; do tools/build $t; echo "tools/build $t exit=$?"; ls build/$t 2>&1; done
tools/difftest native > ../exit2-difftest.out 2>&1; echo "tools/difftest native after it exit=$?"; tail -1 ../exit2-difftest.out; grep -ci skip ../exit2-difftest.out
tests/stale-compiler native > ../exit2-stale.out 2>&1; echo "tests/stale-compiler native exit=$?"; tail -1 ../exit2-stale.out
git checkout -q src/string/string.lu; git status --porcelain; echo "restored, git status exit=$?"
