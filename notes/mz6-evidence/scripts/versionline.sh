#!/usr/bin/env bash
# mz6: the version-line check refuses (P7). In a clean checkout at the
# pin commit $1: the real digest, and the pin's version_line with the pin
# sha's last digit changed (working tree only, restored after).
# tools/fetch-toolchain must refuse naming both lines; it has staged the
# driver by then, so tools/build then finds a driver at the pin's path
# and must refuse it at its own pin check, naming both lines, leaving no
# build/<tier>/. Then the control: the real pin builds.
#   versionline.sh <commit>
set -uo pipefail
L=$HOME/lanes/mz6
co=$L/digest; arc=$L/dl/wolf-0.2.34-x86_64-unknown-linux-gnu.tar.gz
cd "$co" || exit 2
git fetch -q origin mz6 || exit 2
git checkout -q "$1" || exit 2
git clean -xfdq
echo "== commit $(git rev-parse HEAD) date=$(date -u +%FT%TZ)"
real=$(sed -n 's/^version_line = "\(.*\)"$/\1/p' wolf-toolchain.toml)
planted=${real%?)}d\); [ "$real" = "${real%c)}c)" ] || { echo "versionline.sh: the line does not end in c)"; exit 2; }
echo "pin version_line:     $real"
echo "planted version_line: $planted"
[ -n "$real" ] && [ "$real" != "$planted" ] || { echo "versionline.sh: no plant"; exit 2; }
sed -i "s/^version_line = \"$real\"\$/version_line = \"$planted\"/" wolf-toolchain.toml
git diff
echo; echo "#### 1. the planted line, the real 0.2.34 archive (WOLF_ARCHIVE)"
WOLF_ARCHIVE=$arc tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
echo "staged: $(ls .wolf-bin | tr '\n' ' ')"
.wolf-bin/wolf-0.2.34-x86_64-unknown-linux-gnu/wolf --version | head -1
echo; echo "#### 2. tools/build with that driver at the pin's path, an earlier build's output in build/<tier>/"
for t in native release; do
  mkdir -p build/$t; echo "left by an earlier build" > build/$t/left-by-an-earlier-build
  tools/build $t; echo "build $t exit=$?"
  ls -d build/$t 2>&1; echo "ls build/$t exit=$?"
done
echo; echo "#### 3. tools/build with WOLF naming the same driver"
WOLF=$co/.wolf-bin/wolf-0.2.34-x86_64-unknown-linux-gnu/wolf tools/build native; echo "build native (WOLF) exit=$?"
git checkout -q wolf-toolchain.toml; git status --porcelain; echo "plant restored, git status exit=$?"
echo; echo "#### 4. control: the real pin, the driver already staged"
tools/build native; echo "build native exit=$?"
git clean -xfdq; git status --porcelain; echo "git status exit=$?"
