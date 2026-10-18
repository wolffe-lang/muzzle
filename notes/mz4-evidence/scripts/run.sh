#!/usr/bin/env bash
# mz4: build and difftest both tiers from a clean tree, keep the objects.
#   run.sh <checkout> <label> <archive>
set -uo pipefail
co=$1; label=$2; arc=$3
keep=$HOME/lanes/mz4/objs/$label
rm -rf "$keep"; mkdir -p "$keep"
cd "$co" || exit 2
git clean -xfdq
echo "== commit $(git rev-parse HEAD) label=$label host=$(uname -srm) date=$(date -u +%FT%TZ)"
echo "== archive $(sha256sum "$arc")"
git status --porcelain; echo "git status exit=$? (clean tree above this line = nothing listed)"
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
ls .wolf-bin
for t in native release; do
  tools/build $t; echo "build $t exit=$?"
  tools/difftest $t; echo "difftest $t exit=$?"
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
