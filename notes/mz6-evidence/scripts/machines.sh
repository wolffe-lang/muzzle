#!/usr/bin/env bash
# mz6: what the checked machine and lupin 0.1.55 answer for muzzle, and
# exit 2 at 0.2.34 (a compile error planted, never committed).
set -uo pipefail
L=$HOME/lanes/mz6
X=$L/x; W=$X/wolf-0.2.34-x86_64-unknown-linux-gnu/wolf; LU=$X/lupin-0.1.55-x86_64-unknown-linux-gnu/lupin
cd $L/muzzle/src || exit 2
echo "== commit $(git rev-parse HEAD) date=$(date -u +%FT%TZ)"
sha256sum $L/dl/*.tar.gz
$W --version; $LU --version
echo "std/STD-REV: $(cat $X/wolf-0.2.34-x86_64-unknown-linux-gnu/std/STD-REV)"
echo; echo "#### the checked machine: wolf conform-run --json --checked"
echo "\$ $W conform-run --json --checked muzzle.lu"; $W conform-run --json --checked muzzle.lu; echo "exit=$?"
echo; echo "#### wolf run --checked (the NATIVE build, not the checked machine)"
echo "\$ $W run --checked muzzle.lu"; $W run --checked muzzle.lu; echo "exit=$?"
echo; echo "#### lupin 0.1.55"
echo "\$ $LU run muzzle.lu"; $LU run muzzle.lu; echo "exit=$?"
echo "\$ $LU conform-run muzzle.lu"; $LU conform-run muzzle.lu; echo "exit=$?"
echo; echo "#### exit 2: a type error planted in src/string/string.lu (restored after)"
cd ..
printf '\nfn mz6_planted() -> i32 {\n    return "not an i32"\n}\n' >> src/string/string.lu
git diff --stat
for t in native release; do tools/build $t; echo "tools/build $t exit=$?"; ls -A build/$t 2>&1; echo "ls build/$t exit=$?"; done
tools/difftest native > $L/logs/exit2-difftest.out 2>&1; echo "tools/difftest native after it exit=$?"; tail -1 $L/logs/exit2-difftest.out; echo "lines saying skip: $(grep -ci skip $L/logs/exit2-difftest.out)"
tools/difftest release > $L/logs/exit2-difftest-release.out 2>&1; echo "tools/difftest release after it exit=$?"; tail -1 $L/logs/exit2-difftest-release.out; echo "lines saying skip: $(grep -ci skip $L/logs/exit2-difftest-release.out)"
tests/stale-compiler native > $L/logs/exit2-stale.out 2>&1; echo "tests/stale-compiler native exit=$?"; tail -1 $L/logs/exit2-stale.out
git checkout -q src/string/string.lu; git status --porcelain; echo "restored, git status exit=$?"
