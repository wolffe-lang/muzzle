#!/usr/bin/env bash
# mz6: build and difftest both tiers from a clean tree, keep the objects.
#   run.sh <checkout> <label> <archive>
set -uo pipefail
L=$HOME/lanes/mz6
co=$1; label=$2; arc=$3
keep=$L/objs/$label
rm -rf "$keep"; mkdir -p "$keep"
cd "$co" || exit 2
git clean -xfdq
echo "== commit $(git rev-parse HEAD) label=$label tree=$co host=$(uname -srm) date=$(date -u +%FT%TZ)"
echo "== archive $(sha256sum "$arc")"
echo "== this shell's signal masks: $(grep -E '^Sig(Blk|Ign)' /proc/self/status | tr '\n\t' '  ')"
git status --porcelain; echo "git status exit=$? (clean tree above this line = nothing listed)"
grep -E '^(version|rev|version_line|x86_64-unknown-linux-gnu) = ' wolf-toolchain.toml
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
ls .wolf-bin
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
  taskset -c 0-3 tools/difftest $t >"$L/logs/$label-taskset-$t.out" 2>&1; echo "difftest $t under taskset -c 0-3 exit=$?"; grep -c '^PASS ' "$L/logs/$label-taskset-$t.out"; tail -1 "$L/logs/$label-taskset-$t.out"
  mkdir -p "$keep/$t" "$keep/$t-raw"
  for o in build/$t/*.o; do
    b=$(basename "$o"); cp "$o" "$keep/$t-raw/$b"
    objcopy --strip-debug "$o" "$keep/$t/$b.s"
  done
  (cd "$keep/$t" && sha256sum *)
  for o in build/$t/muzzle.*.o; do
    p=$(readelf --debug-dump=info "$o" 2>/dev/null | grep -m1 DW_AT_producer | sed 's/.*: //')
    echo "producer $(basename "$o"): ${p:-<none>}"
  done
done
echo "== after build and difftest:"
git status --porcelain; echo "git status exit=$?"
