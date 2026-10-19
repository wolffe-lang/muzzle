#!/usr/bin/env bash
# mz5: the digest check refuses (P6). In a clean checkout at the pin
# commit $1: the pin's linux x86-64 digest with its last hex digit
# changed (in the working tree, restored after), offered the real 0.2.33
# archive, then fetched from the release as CI does; then the real pin
# offered the 0.2.32 archive. Each must exit 1 naming both digests and
# stage no driver.
#   digest.sh <commit>
set -uo pipefail
co=$HOME/lanes/mz5/digest; dl=$HOME/lanes/mz5/dl
cd "$co" || exit 2
git fetch -q origin mz5 || exit 2
git checkout -q "$1" || exit 2
git clean -xfdq
echo "== commit $(git rev-parse HEAD) date=$(date -u +%FT%TZ)"
real=$(sed -n 's/^x86_64-unknown-linux-gnu = "\(.*\)"$/\1/p' wolf-toolchain.toml)
planted=${real%?}9; [ "${real: -1}" = 9 ] && planted=${real%?}8
echo "pin digest:     $real"
echo "planted digest: $planted"
[ -n "$real" ] && [ "$real" != "$planted" ] || { echo "digest.sh: no plant"; exit 2; }
staged() { echo "staged: $(ls .wolf-bin 2>/dev/null | tr '\n' ' ')"; ls -d .wolf-bin/wolf-0.2.33-x86_64-unknown-linux-gnu 2>&1; echo "driver dir exit=$?"; }
sed -i "s/^x86_64-unknown-linux-gnu = \"$real\"\$/x86_64-unknown-linux-gnu = \"$planted\"/" wolf-toolchain.toml
git diff
echo; echo "#### 1. the planted digest, the real 0.2.33 archive (WOLF_ARCHIVE)"
WOLF_ARCHIVE=$dl/wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
staged
tools/build native; echo "build native after it exit=$?"
rm -rf .wolf-bin
echo; echo "#### 2. the planted digest, fetched from the release (CI's path)"
tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
staged
rm -rf .wolf-bin
git checkout -q wolf-toolchain.toml; git status --porcelain; echo "plant restored, git status exit=$?"
echo; echo "#### 3. the real pin, the 0.2.32 archive offered for it (WOLF_ARCHIVE)"
WOLF_ARCHIVE=$dl/wolf-0.2.32-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
staged
rm -rf .wolf-bin
echo; echo "#### 4. control: the real pin, the real archive"
WOLF_ARCHIVE=$dl/wolf-0.2.33-x86_64-unknown-linux-gnu.tar.gz tools/fetch-toolchain; echo "fetch-toolchain exit=$?"
staged
git clean -xfdq; git status --porcelain; echo "git status exit=$?"
