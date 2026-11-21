#!/bin/sh
# Builds build/mgs3danim.asi from mgs3danim.c with LLVM (clang + lld-link + llvm-dlltool), on Linux or Windows.
set -e
cd "$(dirname "$0")"
for d in kernel32 user32 gdi32; do llvm-dlltool -m i386:x86-64 -d $d.def -l $d.lib; done
clang --target=x86_64-pc-windows-msvc -O2 -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe -fno-math-errno -Wall -Wextra -c ../mgs3danim.c -o mgs3danim.obj
lld-link /dll /nodefaultlib /entry:DllMain /Brepro /out:mgs3danim.asi mgs3danim.obj kernel32.lib user32.lib gdi32.lib
echo "built: $(pwd)/mgs3danim.asi"
