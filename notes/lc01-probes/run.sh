#!/bin/bash
# lc01 probes: wolf 0.2.24 by digest, kasumi. Every command echoed with its exit status.
W=~/lanes/lc01/dist/wolf/wolf-0.2.24-x86_64-unknown-linux-gnu/wolf
cd ~/lanes/lc01/probes
run() { echo "\$ $*"; "$@"; echo "exit=$?"; }
echo "== host"; run $W --version; run clang --version; uname -r
echo "== q1: the address of a module var"
for s in a b c d; do echo "-- q1/$s"; grep -nE 'errno_v as|&|addr_of' q1/$s/lib.lu; run $W build q1/$s/lib.lu --target x86_64-unknown-none --emit=obj -o q1/$s/lib.o; done
for tier in native release; do
  fl=""; [ $tier = release ] && fl=--release
  echo "== q2 ($tier): C-named exports, a listed .S syscall, wolf_trap, a module var"
  (cd q2 && rm -f *.o q2 && run $W build lib.lu --target x86_64-unknown-none --emit=obj $fl -o lib.o && command ls *.o && echo "-- nm lib.o" && nm lib.o && run clang -c crt0.S -o crt0.o && run clang -c main.c -O1 -ffreestanding -fno-builtin -o main.o && run clang -nostdlib -static -o q2 crt0.o main.o lib.o lib.asm-sys.o && run ./q2)
  echo "== q3 ($tier): a copy loop named memcpy"
  (cd q3 && rm -f *.o && run $W build lib.lu --target x86_64-unknown-none --emit=obj $fl -o lib.o && echo "-- nm lib.o" && nm lib.o)
done
