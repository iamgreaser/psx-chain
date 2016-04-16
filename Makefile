CROSSPREFIX=mipsel-none-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs

ASFLAGS = -g -msoft-float

CFLAGS = -g -c -O3 -flto -pipe \
	-fomit-frame-pointer -fno-stack-protector \
	-mno-check-zero-division \
	-msoft-float -nostdlib -mips1 -march=3000 -mtune=3000 \
	-Isrc -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

#LDFLAGS = -g -O2 -flto \

#LDFLAGS = -g -O1 -flto -Wl,-T,newlink.ld -Wl,-Ttext-segment=0x8000F800 -pipe \
#

LDFLAGS = -g -O3 -flto -Wl,-T,link.ld -Wl,-Ttext-segment=0x8000F800 -pipe \
	-mtune=3000 -march=3000 \
	\
	-msoft-float \
	-L/usr/local/mipsel-none-elf/lib/soft-float/

LIBS = -lm -lc -lgcc

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
	$(OBJDIR)/GL/ctab.o \
	$(OBJDIR)/GL/draw.o \
	$(OBJDIR)/GL/enable.o \
	$(OBJDIR)/GL/error.o \
	$(OBJDIR)/GL/list.o \
	$(OBJDIR)/GL/matrix.o \
	$(OBJDIR)/GL/tex.o \
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
	#$(CROSS_OBJCOPY) -O binary -j .text.head $(OBJDIR)/$(EXE_NAME).elf $(OBJDIR)/$(EXE_NAME).head
	$(CROSS_OBJCOPY) -O binary $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe
	#$(CROSS_OBJCOPY) -O elf32-littlemips $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/head.o: $(SRCDIR)/head.S $(INCLUDES)
	$(CROSS_AS) -o $@ $(ASFLAGS) $<

