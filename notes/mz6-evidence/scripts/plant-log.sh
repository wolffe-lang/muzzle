#!/usr/bin/env bash
# mz6: a CI plant run's evidence: the run, each job's step conclusions,
# then `gh run view --log-failed`. Read from nomad-1 (gh only, no build).
#   plant-log.sh <run id> <what was planted>
set -uo pipefail
R=wolffe-lang/muzzle; id=$1; what=$2
echo "# mz6 plant: $what"
echo "# run $id, read $(date -u +%FT%TZ):"
gh run view "$id" -R $R --json databaseId,event,headBranch,headSha,status,conclusion -q '"run \(.databaseId) event=\(.event) branch=\(.headBranch) head=\(.headSha) \(.status) \(.conclusion)"'
gh run view "$id" -R $R --json jobs -q '.jobs[] | "job \(.name) = \(.conclusion): " + ([.steps[] | "\(.name)=\(.conclusion)"] | join(", "))'
echo "# gh run view $id --log-failed:"
gh run view "$id" -R $R --log-failed
echo "# gh run view $id --log-failed exit=$?"
