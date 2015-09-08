CROSSPREFIX=mipsel-none-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs

ASFLAGS = -g -G0

CFLAGS = -g -c -O3 -flto -pipe \
	-fomit-frame-pointer -fno-stack-protector -G0 \
	-mno-check-zero-division \
	-msoft-float -nostdlib -mips1 -march=3000 -mtune=3000 \
	-Isrc -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

#LDFLAGS = -g -O2 -flto \

# LTO seems to break ATM on -O2, -O3, -Os
# but when posting every -f flag that -O2 uses it works?
#LDFLAGS = -g -Wl,-T,link.ld -O1 -flto

LDFLAGS = -g -Wl,-T,link.ld -O1 -flto -pipe \
	-mtune=3000 -march=3000 \
	\
	-funroll-loops \
	\
	-fthread-jumps \
	-falign-functions \
	-falign-jumps \
	-falign-loops \
	-falign-labels \
	-fcaller-saves \
	-fcrossjumping \
	-fcse-follow-jumps \
	-fcse-skip-blocks \
	-fdelete-null-pointer-checks \
	-fdevirtualize \
	-fexpensive-optimizations \
	-fgcse \
	-fgcse-lm \
	-finline-small-functions \
	-findirect-inlining \
	-fipa-cp \
	-fipa-sra \
	-foptimize-sibling-calls \
	-foptimize-strlen \
	-fpartial-inlining \
	-fpeephole2 \
	-freorder-blocks \
	-freorder-blocks-and-partition \
	-freorder-functions \
	-frerun-cse-after-loop \
	-fsched-interblock \
	-fsched-spec \
	-fschedule-insns \
	-fschedule-insns2 \
	-fstrict-aliasing \
	-fstrict-overflow \
	-ftree-builtin-call-dce \
	-ftree-switch-conversion \
	-ftree-tail-merge \
	-ftree-pre \
	-ftree-vrp \
	\
	-finline-functions \
	-fpredictive-commoning \
	-fgcse-after-reload \
	-ftree-loop-distribute-patterns \
	-ftree-slp-vectorize \
	-fvect-cost-model \
	\
	-msoft-float \
	-L/usr/local/mipsel-none-elf/lib/soft-float/ \
	-L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/

LIBS = -lm -lc -lgcc -lnullmon

# stuff omitted:
# O2:
# O3:
# -funswitch-loops - slows things down
# -fipa-cp-clone - also slows things down
#

EXE_NAME=boot
ISO_NAME=chaintest

OBJDIR = obj
SRCDIR = src
INCLUDES = src/psx.h src/common.h src/GL/gl.h src/GL/intern.h

OBJS = $(OBJDIR)/head.o \
	\
	$(OBJDIR)/nullmon-part.o \
	\
	$(OBJDIR)/dma.o \
	$(OBJDIR)/fix.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/gte.o \
	$(OBJDIR)/joy.o \
	\
	$(OBJDIR)/GL/gl.o \
	\
	$(OBJDIR)/GL/begin.o \
	$(OBJDIR)/GL/clear.o \
	$(OBJDIR)/GL/draw.o \
	$(OBJDIR)/GL/enable.o \
	$(OBJDIR)/GL/error.o \
	$(OBJDIR)/GL/list.o \
	$(OBJDIR)/GL/matrix.o \
	$(OBJDIR)/GL/viewport.o \
	\
	$(OBJDIR)/main.o


all: $(EXE_NAME).exe $(ISO_NAME).cue

clean:
	$(RM_F) $(OBJS)

$(ISO_NAME).cue: $(ISO_NAME) tools/iso2raw
	./tools/iso2raw tools/rawhead2.bin $(ISO_NAME)

tools/iso2raw: tools/iso2raw.c
	$(CC) -o tools/iso2raw tools/iso2raw.c

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	$(CROSS_OBJCOPY) -O binary $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/head.o: $(SRCDIR)/head.S $(INCLUDES)
	$(CROSS_AS) -o $@ $(ASFLAGS) $<

