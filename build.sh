#!/bin/sh
#export PATH=${PATH}:/usr/local/libexec/gcc/mipsel-none-elf/4.7.0/

# REMEMBER TO CHANGE system.cnf IF YOU MODIFY EXE_NAME
export EXE_NAME=boot
export ISO_NAME=chaintest

#echo as-dbg && mipsel-none-elf-as -G1 -o obj/head-dbg.o --defsym _LOADABLE_SIZE=0x800 head.S && \
#echo ld-dbg && true mipsel-none-elf-gcc -msoft-float -o obj/${EXE_NAME}.dbg obj/head-dbg.o obj/main.o -L/usr/local/mipsel-none-elf/lib/soft-float/ -L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ -lm -lc -lgcc -lnosys && \

echo cc && mipsel-none-elf-gcc -c -fomit-frame-pointer -fno-stack-protector -G1 -O3 -msoft-float -nostdlib -mips1 -march=3000 -o obj/main.o main.c && \
echo as && mipsel-none-elf-as -G1 -o obj/head.o head.S && \
echo ld && mipsel-none-elf-ld -T link.ld -o obj/${EXE_NAME}.elf obj/head.o obj/main.o -L/usr/local/mipsel-none-elf/lib/soft-float/ -L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ -lm -lc -lgcc -lnosys && \
echo strip && mipsel-none-elf-strip -R .reginfo -R .pdr -R .text.startup -R .eh_frame -R .comment -R .gnu.attributes -R .rel.dyn obj/${EXE_NAME}.elf && \
echo objcopy && mipsel-none-elf-objcopy -O binary obj/${EXE_NAME}.elf obj/${EXE_NAME}.tmp && \
cp obj/${EXE_NAME}.tmp ${EXE_NAME}.exe && \
cc -o tools/iso2raw tools/iso2raw.c && \
mkisofs -o ${ISO_NAME} system.cnf ${EXE_NAME}.exe && \
./tools/iso2raw tools/rawhead2.bin ${ISO_NAME} && \
true
#~/Downloads/mednafen/src/mednafen ${EXE_NAME}.exe
#clang33 -mattr=-condmov -fomit-frame-pointer -G1 -Os -nostdlib -target mipsel -S -o obj/${EXE_NAME}.base.S main.c && \
#python fuckgas.py && \
#python fuckbinutils.py && \


