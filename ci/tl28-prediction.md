# tl28 — the prediction, before the first change (ruling #92)

Written before any tl28 change to this repository is committed, against
trunk `ebf23e9`.

**Before** (the ten completed push/pull_request runs of `ci` up to
39312514613): one job on ubuntu, the difftest on both compiling tiers;
**0.7 job-minutes** a run (median 0.6) and **0.5 minutes of wall**. Its one
suite cannot be split by path: tl28 gives it no plan and no pr tier.

**After, predicted:**

1. Every run, a pull request's push included, is `tier full`: the difftest
   (0.5 to 1 job-minute) and the verdict (under 1, the wiring test inside
   it): **1 to 2 job-minutes, about 1.5 minutes of wall**. The verdict
   costs one short ubuntu job a run; nothing is saved here and nothing was
   to be.
2. A superseded pull-request run is cancelled by the next push.
3. The planted break (a difftest step made to fail) reds `tier full`: the
   verdict reports `difftest failure`.
4. The nightly runs `tier full` on trunk; a red one opens one issue.
