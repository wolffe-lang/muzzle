#!/usr/bin/env bash
# lc02's detached job wrapper on hasu (the lane rules, 2026-11-01 addition):
# one long flock per lock name, the block name outside, never a retry loop.
#   setsid -f bash run-job.sh NAME A|B INNER-SCRIPT [ARGS...]
# Writes jobs/NAME.pid (this shell's pid), logs/NAME.log, jobs/NAME.exit.
set -u
L=/home/mfwolffe/lanes/lc02
name=$1; blk=$2; inner=$3; shift 3
log=$L/logs/$name.log
echo $$ >"$L/jobs/$name.pid"
rm -f "$L/jobs/$name.exit"
{
  echo "QUEUED $(date -u +%FT%TZ) job=$name block=$blk pid=$$ inner=$inner $*"
  flock -w 21600 /home/mfwolffe/lanes/hasu-block-$blk.lock \
    flock -w 21600 /home/mfwolffe/lanes/hasu-$blk.lock \
    env LC02_BLOCK=$blk bash "$inner" "$@"
  rc=$?
  echo "DONE $(date -u +%FT%TZ) job=$name exit=$rc"
} >>"$log" 2>&1
echo "$rc" >"$L/jobs/$name.exit"
