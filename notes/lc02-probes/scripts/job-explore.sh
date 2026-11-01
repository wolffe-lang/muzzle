#!/usr/bin/env bash
# lc02 exploration job, run by run-job.sh inside both block locks:
# a worktree pinned to SHA, then one container on the block's CPUs that
# runs ctr-explore.sh (every step runs; each prints END <step> exit=N).
#   job-explore.sh SHA STAMP
set -u
L=/home/mfwolffe/lanes/lc02
sha=$1; stamp=$2
case ${LC02_BLOCK:?} in A) cpus=0-5;; B) cpus=6-11;; *) echo "bad block"; exit 2;; esac
export PATH=$HOME/lanes/hasu-env/bin:$HOME/lanes/hasu-env/profile/bin:$PATH
git -C "$L/muzzle" fetch -q origin '+refs/heads/*:refs/remotes/origin/*' || { echo "fetch failed"; exit 2; }
# Local WIP commits come as a bundle (never pushed): xfer.bundle.
if [ -f "$L/xfer.bundle" ]; then
  git -C "$L/muzzle" fetch -q "$L/xfer.bundle" '+refs/heads/*:refs/remotes/bundle/*' || { echo "bundle fetch failed"; exit 2; }
fi
w="$L/build-$sha-$stamp"
if [ ! -d "$w" ]; then
  git -C "$L/muzzle" worktree add -q --detach "$w" "$sha" || { echo "worktree failed"; exit 2; }
fi
[ "$(git -C "$w" rev-parse HEAD)" = "$sha" ] || { echo "worktree at the wrong commit"; exit 2; }
out="$L/out-$stamp"; mkdir -p "$out"
echo "explore: sha=$sha block=$LC02_BLOCK cpus=$cpus out=$out"
echo "$sha" >"$w/.lc02-sha"
podman run --rm --memory 12g --network none -e QUICK="${QUICK:-}" -e PROBES="${PROBES:-}" -e GPROBES="${GPROBES:-}" -e STOP_ON_FAIL="${STOP_ON_FAIL:-}" \
  -v "$w":/work -v "$L/archives":/archives:ro -v "$L/probes":/probes:ro \
  -v "$L/hasu":/scripts:ro -v "$out":/out -w /work \
  localhost/px75-ubuntu:2 bash -c "taskset -c $cpus bash /scripts/${CTR:-ctr-explore.sh}; exit \$?"
echo "explore: container exit=$?"
