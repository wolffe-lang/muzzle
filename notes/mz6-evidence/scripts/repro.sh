#!/usr/bin/env bash
# mz6: two cross-checks of the run logs. (1) the 0.2.33 baseline's 12
# stripped hashes against mz5's committed pin-0.2.33.log (the build
# reproduces across days and trees); (2) the 18 PASS lines of run A
# against run B.
#   repro.sh <logA> <logB> <mz5 log>
set -uo pipefail
A=$1; B=$2; M=$3
h() { grep -E '^[0-9a-f]{64}  ' "$1"; }
p() { grep -E '^PASS ' "$1" | grep -v taskset; }
echo "== (1) stripped hashes: $(basename "$A") against mz5's $(basename "$M")"
echo "lines: $(h "$A" | wc -l) and $(h "$M" | wc -l)"
diff <(h "$M") <(h "$A"); echo "diff exit=$?"
echo "== (2) PASS lines: $(basename "$A") against $(basename "$B")"
echo "lines: $(p "$A" | wc -l) and $(p "$B" | wc -l)"
diff <(p "$A") <(p "$B"); echo "diff exit=$?"
echo "== (2b) stripped hashes: $(basename "$A") against $(basename "$B")"
diff <(h "$A") <(h "$B"); echo "diff exit=$?"
