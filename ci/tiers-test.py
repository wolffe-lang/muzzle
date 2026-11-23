#!/usr/bin/env python3
"""ci/tiers-test.py — the wiring's own proof (tl28). python3 and bash only.

    ci/tiers-test.py

muzzle has one tier (ci/tiers.py), so this proves the shape around it:

  T4  .github/workflows/ci.yml: a push on trunk alone, the pull request, the
      nightly, a dispatch whose tier input is full alone; cancel-in-progress
      for pull requests only, every other run a group of its own run id; the
      `tier full` job last but the nightly's issue, needing every job,
      running on `!cancelled()`, this file first and the verdict after; a
      red nightly, and only a red nightly, runs ci/nightly-red.sh.
  T6  the verdict: green when every job succeeded; red for a job failed,
      skipped or cancelled, and for a tier job that needs no job.
  T8  ci/nightly-red.sh against a stand-in `gh`: no open issue opens one,
      labelled, under the one title; an open one gets a comment and no new
      issue; a label that already exists is no failure.

Exit 0 when every check passes, 1 otherwise.
"""
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VERDICT = os.path.join(ROOT, "ci", "tiers.py")
RED = os.path.join(ROOT, "ci", "nightly-red.sh")
WORKFLOW = os.path.join(ROOT, ".github", "workflows", "ci.yml")
fails = 0


def say(ok, what):
    global fails
    print("tiers-test: %s %s" % ("PASS" if ok else "FAIL", what))
    fails += not ok


wf = open(WORKFLOW, encoding="utf-8").read()
JOBS = re.findall(r"^  ([A-Za-z0-9_-]+):\s*$", wf[wf.index("\njobs:\n"):], re.M)
EVERY = [j for j in JOBS if j not in ("plan", "tier", "nightly-red")]
blocks = {}
for j in JOBS:
    m = re.search(r"^  %s:\s*\n(.*?)(?=^  [A-Za-z0-9_-]+:\s*$|\Z)" % re.escape(j), wf, re.M | re.S)
    blocks[j] = m.group(1) if m else ""

# --- T4 the wiring ---------------------------------------------------------------
say(JOBS == ["plan", "difftest", "tier", "nightly-red"], "T4 the workflow's jobs: %s" % " ".join(JOBS))
t = blocks["tier"]
say("    name: tier full\n" in t, "T4 the verdict job is named tier full")
needs = re.search(r"^    needs: \[(.*)\]$", t, re.M)
say(needs is not None and sorted(x.strip() for x in needs.group(1).split(",")) == sorted(EVERY),
    "T4 the tier job needs every job (%s)" % (needs.group(1) if needs else "none"))
say(re.findall(r"^    if: (.*)$", t, re.M) == ["${{ (github.event_name != 'pull_request' || contains(github.event.pull_request.labels.*.name, 'full-matrix') || contains(github.event.pull_request.labels.*.name, 'ci') || contains(github.event.pull_request.labels.*.name, 'release-matrix')) && (!cancelled()) }}"], "T4 the tier job runs on !cancelled(), never always()")
say("        run: python3 ci/tiers-test.py\n" in t and "        run: python3 ci/tiers.py verdict\n" in t
    and t.index("ci/tiers-test.py") < t.index("ci/tiers.py verdict") and "          NEEDS: ${{ toJSON(needs) }}\n" in t,
    "T4 the tier job proves the wiring, then checks the verdict over toJSON(needs)")
n = blocks["nightly-red"]
say("    needs: tier\n" in n and "    if: ${{ failure() && github.event_name == 'schedule' }}\n" in n
    and "      issues: write\n" in n and "        run: ci/nightly-red.sh\n" in n and "WORKFLOW: ${{ github.workflow }}" in n,
    "T4 a red nightly, and only a red nightly, runs ci/nightly-red.sh with issues: write")
say("  cancel-in-progress: ${{ github.event_name == 'pull_request' }}\n" in wf, "T4 cancel-in-progress for pull requests only")
g = re.search(r"^concurrency:\n  group: (.*)$", wf, re.M)
g = g.group(1) if g else ""
say(g.endswith("|| format('ci-{0}-{1}', github.event_name, github.run_id) }}") and "format('ci-pr-{0}', github.head_ref)" in g,
    "T4 a pull request's runs share a group per branch; every other run has a group of its own run id")
say(re.search(r"^  push:\n    branches: \[trunk\]\n", wf, re.M) is not None, "T4 a push runs on trunk alone")
say(re.search(r"^  pull_request:\n", wf, re.M) is not None, "T4 a pull request runs")
say(re.search(r"^  schedule:\n(\s+#.*\n)*\s+- cron: ", wf, re.M) is not None, "T4 the nightly runs")
say(re.search(r"^  workflow_dispatch:\n    inputs:\n      tier:\n", wf, re.M) is not None and "        options: [full]\n" in wf,
    "T4 a dispatch's tier input is full alone")
say("    runs-on: ubuntu-24.04\n" in blocks["difftest"] and "        tier: [native, release]\n" in blocks["difftest"],
    "T4 the suite is unchanged: ubuntu, both compiling tiers")


# --- T6 the verdict -------------------------------------------------------------
def verdict(needs):
    r = subprocess.run([sys.executable, VERDICT, "verdict"], capture_output=True, text=True,
                       env=dict(os.environ, NEEDS=json.dumps(needs)))
    return r.returncode


ok = {"result": "success", "outputs": {}}
for name, needs, want in [("every job succeeded", {"difftest": ok}, 0),
                          ("a job failed", {"difftest": {"result": "failure"}}, 1),
                          ("a job was skipped", {"difftest": {"result": "skipped"}}, 1),
                          ("a job was cancelled", {"difftest": {"result": "cancelled"}}, 1),
                          ("the tier job needs no job", {}, 1)]:
    rc = verdict(needs)
    say(rc == want, "T6 %s: exit %d" % (name, rc))

# --- T8 the nightly's issue -------------------------------------------------------
FAKE_GH = r'''#!/usr/bin/env bash
printf '%s\n' "$*" >> "$FAKE_LOG"
case "$1 $2" in
  "label create") exit "${FAKE_LABEL_RC:-0}" ;;
  "issue list") printf '%s' "${FAKE_EXISTING:-}"; [ -n "${FAKE_EXISTING:-}" ] && echo; exit 0 ;;
esac
exit 0
'''


def nightly_red(existing="", label_rc=0, **env_more):
    d = tempfile.mkdtemp(prefix="tiers-test-gh-")
    try:
        with open(os.path.join(d, "gh"), "w") as f:
            f.write(FAKE_GH)
        os.chmod(os.path.join(d, "gh"), 0o755)
        log = os.path.join(d, "log")
        env = dict(os.environ, PATH=d + os.pathsep + os.environ.get("PATH", ""), FAKE_LOG=log, FAKE_EXISTING=existing,
                   FAKE_LABEL_RC=str(label_rc), REPO="wolffe-lang/muzzle", WORKFLOW="ci",
                   RUN_URL="https://github.com/wolffe-lang/muzzle/actions/runs/1")
        env.update(env_more)
        r = subprocess.run(["bash", RED], capture_output=True, text=True, env=env)
        calls = open(log).read().splitlines() if os.path.exists(log) else []
        return r.returncode, calls
    finally:
        shutil.rmtree(d, ignore_errors=True)


TITLE = "nightly red: ci on trunk"
rc, calls = nightly_red()
creates = [c for c in calls if c.startswith("issue create")]
say(rc == 0 and len(creates) == 1 and "--title %s" % TITLE in creates[0] and "--label nightly-red" in creates[0]
    and not [c for c in calls if c.startswith("issue comment")], "T8 no open issue: one is opened, labelled nightly-red, titled '%s'" % TITLE)
rc, calls = nightly_red(existing="41")
say(rc == 0 and [c for c in calls if c.startswith("issue comment 41 ")] and not [c for c in calls if c.startswith("issue create")],
    "T8 an open issue: a comment on it, no new issue")
rc, calls = nightly_red(label_rc=1)
say(rc == 0 and [c for c in calls if c.startswith("issue create")], "T8 a label that already exists is no failure")
rc, _ = nightly_red(REPO="")
say(rc != 0, "T8 the script refuses to run without REPO")
say(re.search(r"^name: ci$", wf, re.M) is not None, "T8 the workflow's name is ci, the title's")

# T8, what the reporter writes is public: the issue's title and body, a comment,
# the label's description. The plain rule holds in every run: no lane, ruling,
# crawl or sprint word and no internal tag (two letters and two or three digits). A
# local record also passes PUBLIC_TEXT_REFUSE, a file of further words kept outside
# this repository; a word from that file is reported by its line number, never spelt.
PLAIN = re.compile(r"\b(?:lanes?|rulings?|crawls?|sprints?)\b|\b[a-z]{2}\d{2,3}[a-z]?\b", re.I)
LISTED = []
if os.environ.get("PUBLIC_TEXT_REFUSE"):
    with open(os.environ["PUBLIC_TEXT_REFUSE"], encoding="utf-8") as fh:
        LISTED = [(n, re.compile(r"(?<![A-Za-z0-9_-])%s(?![A-Za-z0-9_-])" % re.escape(w.strip()), re.I))
                  for n, w in enumerate(fh, 1) if w.strip() and not w.startswith("#")]


def _log_lines(x):
    return x.splitlines() if isinstance(x, str) else list(x)


written = "\n".join(c for e in ("", "41") for c in _log_lines(nightly_red(existing=e)[1]) if not c.startswith("issue list"))
said = sorted({m.group(0) for m in PLAIN.finditer(written)}) + ["listed word %d" % n for n, rx in LISTED if rx.search(written)]
say(bool(written) and not said, "T8 nothing the reporter writes in public carries an internal word (%s)%s" % (
    "the plain rule and %d listed words" % len(LISTED) if LISTED else "the plain rule", ": " + ", ".join(said) if said else ""))

print("tiers-test: %s" % ("PASS" if fails == 0 else "FAIL (%d)" % fails))
sys.exit(1 if fails else 0)
