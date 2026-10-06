#!/bin/bash
W=~/lanes/lc01/dist/wolf/wolf-0.2.24-x86_64-unknown-linux-gnu/wolf
cd ~/lanes/lc01/probes
run() { echo "\$ $*"; "$@"; echo "exit=$?"; }
for tier in native release; do
  fl=""; [ $tier = release ] && fl=--release
  echo "== q2 ($tier)"
  (cd q2 && rm -f *.o q2 && run $W build lib.lu --target x86_64-unknown-none --emit=obj $fl -o lib.o && command ls *.o && echo "-- nm lib.o" && nm lib.o && run clang -c crt0.S -o crt0.o && run clang -c main.c -O1 -ffreestanding -fno-builtin -fno-stack-protector -o main.o && run clang -nostdlib -static -o q2 crt0.o main.o lib.o lib.asm-sys.o && run ./q2)
  continue; echo "== q3 ($tier): disassembly of memcpy/memset, any call or jmp to a symbol"
  (cd q3 && rm -f *.o && $W build lib.lu --target x86_64-unknown-none --emit=obj $fl -o lib.o 2>/dev/null; objdump -dr --no-show-raw-insn lib.o | grep -E 'call|jmp.*<(mem|wolf)|R_X86' )
  echo "== q4 ($tier): what 'errno_v as *i32' yields (hosted C caller)"
  (cd q4 && rm -f *.o q4 && run $W build lib.lu --target x86_64-unknown-none --emit=obj $fl -o lib.o && nm lib.o && run clang -o q4 main.c lib.o && run ./q4)
done
