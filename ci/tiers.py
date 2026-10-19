#!/usr/bin/env python3
"""ci/tiers.py — whether a CI run ran every job green (tl28, ruling #92).

    ci/tiers.py verdict        (NEEDS: the `tier` job's `toJSON(needs)`)

muzzle has ONE tier, `full`. Its suite is one job, the glibc differential on
both compiling tiers (native and release), on ubuntu, in under two minutes:
there is nothing a path could leave out, so there is no plan, and every run
(a pull request's push, a push to trunk, the nightly, a dispatch) runs it
all. The run's last job is still named `tier full`, so wolf/tools/lane-audit.sh
reads muzzle the way it reads a tiered repository.

`verdict` exits 0 when the tier job needs at least one job and every one of
them succeeded, 1 otherwise, and prints each job's result.
"""
import json
import os
import sys


def verdict(needs):
    if not needs:
        print("tier full: the tier job needs no job")
        return 1
    bad = 0
    print("tier full: %d job(s)" % len(needs))
    for job, r in needs.items():
        got = (r or {}).get("result")
        ok = got == "success"
        bad += not ok
        print("  %-18s %-9s %s" % (job, got, "ok" if ok else "WANTED success"))
    return 1 if bad else 0


def main():
    if sys.argv[1:] != ["verdict"]:
        print("usage: ci/tiers.py verdict", file=sys.stderr)
        sys.exit(2)
    sys.exit(verdict(json.loads(os.environ["NEEDS"])))


if __name__ == "__main__":
    main()
