#!/bin/sh
#export PATH=${PATH}:/usr/local/libexec/gcc/mipsel-none-elf/4.7.0/
mipsel-none-elf-gcc -c -fomit-frame-pointer -fno-stack-protector -G1 -O1 -nostdlib -mips1 -o obj/main.o main.c && \
mipsel-none-elf-as -G1 -o obj/head.o head.S && \
mipsel-none-elf-ld -T link.ld -o obj/pstest.elf obj/head.o obj/main.o -L/usr/local/mipsel-none-elf/lib -L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/ -lc -lgcc -lnosys && \
mipsel-none-elf-strip -R .reginfo -R .pdr -R .text.startup -R .eh_frame obj/pstest.elf && \
mipsel-none-elf-objcopy -O binary obj/pstest.elf obj/pstest.tmp && \
cp obj/pstest.tmp pstest.exe && \
true
#~/Downloads/mednafen/src/mednafen pstest.exe
#clang33 -mattr=-condmov -fomit-frame-pointer -G1 -Os -nostdlib -target mipsel -S -o obj/pstest.base.S main.c && \
#python fuckgas.py && \
#python fuckbinutils.py && \


