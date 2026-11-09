#!/usr/bin/env bash
# lc03, in the container at /work: glibc's column alone for the given
# cases (default: the new ones), as tools/difftest builds and runs it
# (the same flags, a fresh directory per case, stdin from <case>.in or
# /dev/null, TZ=UTC, no core files). Keeps stdout, stderr, the exit
# status and a listing of the files each case left, as the prediction's
# expected output.
set -u
cd /work
step() { local n=$1; shift; echo "BEGIN $n"; "$@"; local rc=$?; echo "END $n exit=$rc"; }
CFLAGS=(-std=c17 -O1 -Wall -Werror -fno-builtin -fno-stack-protector -I /work/tests/lib)
out=/out/expected; mkdir -p "$out"
one() {
  local c=$1 n; n=$(basename "$c" .c)
  local in=/dev/null; [ -f "${c%.c}.in" ] && in="${c%.c}.in"
  clang "${CFLAGS[@]}" "$c" -o "$out/$n.bin" 2>"$out/$n.cc" || { cat "$out/$n.cc"; return 1; }
  rm -rf "$out/$n.d"; mkdir -p "$out/$n.d"
  local t0=$SECONDS
  (cd "$out/$n.d" && ulimit -c 0 && TZ=UTC exec timeout 120 "$out/$n.bin") <"$in" >"$out/$n.out" 2>"$out/$n.err"
  local rc=$?
  echo "$rc" >"$out/$n.status"
  (cd "$out/$n.d" && find . -mindepth 1 | sort) >"$out/$n.files"
  echo "  $n exit=$rc lines=$(wc -l <"$out/$n.out") stderr=$(wc -c <"$out/$n.err") left=$(wc -l <"$out/$n.files") secs=$((SECONDS-t0))"
  rm -rf "$out/$n.d" "$out/$n.bin" "$out/$n.cc"
}
all() {
  local rc=0
  for c in ${CASES:-tests/cases/stdlib_*.c tests/cases/atexit_*.c tests/cases/errno_strerror.c tests/next/*.c}; do
    one "$c" || rc=1
  done
  (cd "$out" && sha256sum *.out *.err *.status *.files >SHA256SUMS)
  return $rc
}
step glibc all
exit 0
