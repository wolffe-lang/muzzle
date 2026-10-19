#!/usr/bin/env bash
# mz5: where the unstripped objects differ, section by section, and the
# producer string each carries. Every section of each object is dumped
# by index with `readelf -x` (raw bytes of any section, loadable or not)
# from A and from B and compared. A first version used `objcopy -O
# binary --only-section`, which writes nothing for a non-loadable
# section, so every .debug_* compared empty against empty: blind. The
# self-check below fails the run if a file that differs as a whole has
# no differing section.
#   sections.sh <labelA> <labelB>
set -uo pipefail
A=$HOME/lanes/mz5/objs/$1; B=$HOME/lanes/mz5/objs/$2
bad=0
echo "== unstripped objects, $1 against $2, section by section (readelf -x by index)"
for t in native-raw release-raw; do
  for f in "$A/$t"/*.o; do b=$(basename "$f"); g="$B/$t/$b"
    na=$(readelf -hW "$f" | sed -n 's/.*Number of section headers: *\([0-9]*\).*/\1/p')
    nb=$(readelf -hW "$g" | sed -n 's/.*Number of section headers: *\([0-9]*\).*/\1/p')
    [ -n "$na" ] && [ "$na" -gt 0 ] || { echo "$t/$b: no section count"; bad=1; continue; }
    nd=0; names=""
    for ((i=1; i<na; i++)); do
      s=$(readelf -SW "$f" | sed -n "s/^ *\[ *$i\] \([^ ]*\) .*/\1/p")
      da=$(readelf -x "$i" "$f" 2>&1); db=$(readelf -x "$i" "$g" 2>&1)
      if [ "$da" != "$db" ]; then nd=$((nd+1)); names="$names $s"; fi
    done
    hd=$(cmp -s <(readelf -SW "$f") <(readelf -SW "$g") && echo same || echo DIFFER)
    whole=$(cmp -s "$f" "$g" && echo same || echo DIFFER)
    echo "$t/$b: section headers $na in A, $nb in B; $nd section(s) differ:${names:- none}; header table $hd; whole file $whole"
    if [ "$whole" = DIFFER ] && [ $nd = 0 ] && [ "$hd" = same ]; then echo "  SELF-CHECK FAIL: the file differs and no section does (the compare is blind)"; bad=1; fi
  done
done
echo "== the producer strings (strings -a | grep wolfgang)"
for f in "$A/native-raw"/muzzle.*.o; do b=$(basename "$f")
  echo "native-raw/$b: $1 '$(strings -a "$f" | grep -m1 wolfgang)' | $2 '$(strings -a "$B/native-raw/$b" | grep -m1 wolfgang)'"
done
echo "== the differing bytes of each native wolf object (cmp -l: count, then the first 12 as offset A->B)"
for f in "$A/native-raw"/muzzle.*.o; do b=$(basename "$f")
  n=$(cmp -l "$f" "$B/native-raw/$b" | wc -l)
  echo "native-raw/$b: $n byte(s) differ: $(cmp -l "$f" "$B/native-raw/$b" | head -12 | awk '{printf "@%d %s->%s ", $1, $2, $3}')"
done
echo "== DW_AT_producer and DW_AT_comp_dir/DW_AT_name of each native wolf object"
for f in "$A/native-raw"/muzzle.*.o; do b=$(basename "$f")
  for x in "$f" "$B/native-raw/$b"; do
    echo "native-raw/$b ($( [ "$x" = "$f" ] && echo A || echo B)): $(readelf --debug-dump=info "$x" 2>/dev/null | grep -E 'DW_AT_(producer|comp_dir|name) ' | head -3 | sed 's/.*DW_AT_/DW_AT_/; s/  */ /g' | tr '\n' ';')"
  done
done
echo "sections.sh self-check: $([ $bad = 0 ] && echo ok || echo FAIL)"
exit $bad
