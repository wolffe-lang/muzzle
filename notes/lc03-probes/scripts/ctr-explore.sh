#!/usr/bin/env bash
# lc03, in the container at /work (a worktree pinned to one commit):
# environment facts, the trunk baseline on both tiers, the wolf probes
# (/probes/p*) linked against the archive just built, and the glibc
# black-box probes (/probes/g*.c). Every step runs and prints
# END <step> exit=N.
set -u
cd /work
step() { local n=$1; shift; echo "BEGIN $n"; "$@"; local rc=$?; echo "END $n exit=$rc"; }
env_facts() {
  clang --version | head -2; getconf GNU_LIBC_VERSION; uname -srm; nproc; id -u
  for t in script nm readelf ar objdump python3 od cmp diff timeout strace; do
    printf '%s: %s\n' "$t" "$(command -v $t || echo absent)"; done
  echo "TZ=${TZ-unset}"; ls -l /etc/localtime 2>&1; ls /usr/share/zoneinfo 2>&1 | head -3
  echo "ulimit -c: $(ulimit -c)"; cat /proc/sys/kernel/core_pattern
  echo "env names: $(env | cut -d= -f1 | sort | tr '\n' ' ')"
  stat -f -c 'fs of /out: %T' /out; stat -f -c 'fs of /work: %T' /work
}
step env env_facts
step fetch env WOLF_ARCHIVE=/archives/wolf-0.2.36-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain
for t in native release; do
  [ -n "${QUICK:-}" ] || step "stale.$t" tests/stale-compiler $t
  step "build.$t" tools/build $t
  [ -n "${QUICK:-}" ] || step "difftest.$t" tools/difftest $t ${CASES:-}
done
W=/work/.wolf-bin/wolf-0.2.36-x86_64-unknown-linux-gnu/wolf
probe() {
  local q=$1 t=$2 n; n=$(basename "$q")
  local o=/out/probes/$n.$t fl=()
  [ "$t" = release ] && fl=(--release)
  rm -rf "$o"; mkdir -p "$o/src"; cp -R "$q"/. "$o/src/"
  (cd "$o/src" && "$W" build lib.lu --target x86_64-unknown-none --emit=obj --deny-warnings "${fl[@]}" -o "$o/lib.o") >"$o/build.log" 2>&1
  local rc=$?
  echo "probe $n $t build exit=$rc"; sed 's/^/  | /' "$o/build.log" | head -60
  [ $rc = 0 ] || return $rc
  local objs; objs=$(ls "$o"/lib*.o)
  echo "  objects: $(cd "$o" && ls lib*.o | tr '\n' ' ')"
  echo "  defined: $(nm --defined-only -g $objs | grep -v ':$' | awk '{print $3}' | sort -u | tr '\n' ' ')"
  echo "  undefined: $(nm -u $objs | grep -v ':$' | awk '{print $2}' | sort -u | tr '\n' ' ')"
  [ -f "$q/main.c" ] || return 0
  clang -std=c17 -O1 -fno-builtin -fno-stack-protector -nostdinc -isystem /work/include \
    -I /work/tests/lib -nostdlib -static /work/build/$t/crt0.o "$q/main.c" $objs /work/build/$t/libmuzzle.a \
    -o "$o/prog" >"$o/link.log" 2>&1
  rc=$?; echo "  link exit=$rc"; sed 's/^/  | /' "$o/link.log" | head -20
  [ $rc = 0 ] || return $rc
  "$o/prog"; rc=$?; echo "  run exit=$rc"; return $rc
}
for q in ${PROBES:-/probes/p*}; do
  [ -d "$q" ] || continue
  for t in native release; do step "probe.$(basename "$q").$t" probe "$q" $t; done
done
gprobe() {
  local g=$1 n; n=$(basename "$g" .c)
  local o=/out/glibc/$n; rm -rf "$o"; mkdir -p "$o"
  clang -std=c17 -O1 -Wall -fno-builtin -fno-stack-protector "$g" -o "$o/prog" || return 1
  (cd "$o" && ./prog >stdout.txt 2>stderr.txt </dev/null); local rc=$?
  echo "  exit=$rc"; cat -v "$o/stdout.txt"; echo; echo "  stderr:"; cat -v "$o/stderr.txt"; return 0
}
for g in /probes/g*.c; do [ -e "$g" ] && step "glibc.$(basename "$g" .c)" gprobe "$g"; done
exit 0
