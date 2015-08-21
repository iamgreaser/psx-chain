#!/bin/sh
#export PATH=${PATH}:/usr/local/libexec/gcc/mipsel-none-elf/4.7.0/
echo cc && mipsel-none-elf-gcc -c -fomit-frame-pointer -fno-stack-protector -G1 -O1 -msoft-float -nostdlib -mips1 -march=2000 -o obj/main.o main.c && \
echo as && mipsel-none-elf-as -G1 -o obj/head.o head.S && \
echo as-dbg && mipsel-none-elf-as -G1 -o obj/head-dbg.o --defsym _LOADABLE_SIZE=0x800 head.S && \
echo ld-dbg && mipsel-none-elf-gcc -msoft-float -o obj/pstest.dbg obj/head-dbg.o obj/main.o -L/usr/local/mipsel-none-elf/lib/soft-float/ -L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ -lm -lc -lgcc -lnosys && \
echo ld && mipsel-none-elf-ld -T link.ld -o obj/pstest.elf obj/head.o obj/main.o -L/usr/local/mipsel-none-elf/lib/soft-float/ -L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ -lm -lc -lgcc -lnosys && \
echo strip && mipsel-none-elf-strip -R .reginfo -R .pdr -R .text.startup -R .eh_frame -R .comment -R .gnu.attributes obj/pstest.elf && \
echo objcopy && mipsel-none-elf-objcopy -O binary obj/pstest.elf obj/pstest.tmp && \
cp obj/pstest.tmp pstest.exe && \
true
#~/Downloads/mednafen/src/mednafen pstest.exe
#clang33 -mattr=-condmov -fomit-frame-pointer -G1 -Os -nostdlib -target mipsel -S -o obj/pstest.base.S main.c && \
#python fuckgas.py && \
#python fuckbinutils.py && \


