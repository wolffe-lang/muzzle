#!/usr/bin/env bash
# lc02's gauntlet, in the container at /work (a worktree pinned to one
# commit). Every step prints `END <step> exit=N`. With STOP_ON_FAIL=1 it
# stops at the first nonzero step (the final gauntlet); without, every
# step runs (development).
set -u
cd /work
stop=${STOP_ON_FAIL:-0}
step() {
  local n=$1; shift
  echo "BEGIN $n"
  "$@"; local rc=$?
  echo "END $n exit=$rc"
  if [ $rc != 0 ] && [ "$stop" = 1 ]; then echo "GAUNTLET stopped at $n exit=$rc"; exit $rc; fi
  return 0
}
facts() { git -C /work rev-parse HEAD 2>/dev/null || cat /work/.lc02-sha 2>/dev/null; clang --version | head -1; getconf GNU_LIBC_VERSION; uname -srm; }
step env facts
step fetch env WOLF_ARCHIVE=/archives/wolf-0.2.36-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain
for t in native release; do
  step "stale.$t" tests/stale-compiler $t
  step "build.$t" tools/build $t
  step "selftest.$t" tests/difftest-selftest $t
  step "difftest.$t" tools/difftest $t
done
echo "GAUNTLET done"
exit 0
