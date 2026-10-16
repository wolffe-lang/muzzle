#!/usr/bin/env bash
# mz3: compare two runs' objects, stripped then raw (the control).
#   cmp.sh <labelA> <labelB>
set -uo pipefail
A=$HOME/lanes/mz3/objs/$1; B=$HOME/lanes/mz3/objs/$2
echo "== stripped objects (objcopy --strip-debug), $1 against $2"
n=0; d=0
for t in native release; do
  for f in "$A/$t"/*; do b=$(basename "$f"); n=$((n+1))
    if [ ! -e "$B/$t/$b" ]; then echo "MISSING $t/$b"; d=$((d+1))
    elif cmp -s "$f" "$B/$t/$b"; then echo "same   $t/$b $(stat -c %s "$f") bytes"
    else echo "DIFFER $t/$b"; d=$((d+1)); fi
  done
  for f in "$B/$t"/*; do [ -e "$A/$t/$(basename "$f")" ] || { echo "EXTRA $t/$(basename "$f")"; d=$((d+1)); }; done
done
echo "== compared $n objects; differ=$d"
echo "== control, that the compare can fire: the same objects NOT stripped"
r=0
for t in native-raw release-raw; do
  for f in "$A/$t"/*; do b=$(basename "$f")
    if cmp -s "$f" "$B/$t/$b"; then echo "same   $t/$b"; else echo "DIFFER $t/$b"; r=$((r+1)); fi
  done
done
echo "== unstripped objects that differ: $r"
