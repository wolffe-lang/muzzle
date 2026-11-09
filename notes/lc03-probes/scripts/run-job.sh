#!/usr/bin/env bash
# lc03's detached job wrapper on hasu (adapted from lc02's): one long
# flock per lock name, in the one order (block, then slot), never a retry
# loop. TERM, INT and HUP end the job with their own non-zero codes, so a
# stopped job never reads as a pass (the 2026-11-05 rule).
#   setsid -f bash run-job.sh NAME A|B|C INNER-SCRIPT [ARGS...]
# Writes jobs/NAME.pid (this shell's pid), logs/NAME.log, jobs/NAME.exit.
set -u
L=/home/mfwolffe/lanes/lc03
name=$1; blk=$2; inner=$3; shift 3
log=$L/logs/$name.log
echo $$ >"$L/jobs/$name.pid"
rm -f "$L/jobs/$name.exit"
stopped() { echo "STOPPED $(date -u +%FT%TZ) job=$name signal=$1" >>"$log"; echo "$2" >"$L/jobs/$name.exit"; exit "$2"; }
trap 'stopped TERM 143' TERM
trap 'stopped INT 130' INT
trap 'stopped HUP 129' HUP
{
  echo "QUEUED $(date -u +%FT%TZ) job=$name block=$blk pid=$$ inner=$inner $*"
  flock -w 21600 /home/mfwolffe/lanes/hasu-block-$blk.lock \
    flock -w 21600 /home/mfwolffe/lanes/hasu-$blk.lock \
    env LC03_BLOCK=$blk bash "$inner" "$@"
  rc=$?
  echo "DONE $(date -u +%FT%TZ) job=$name exit=$rc"
} >>"$log" 2>&1
echo "$rc" >"$L/jobs/$name.exit"
