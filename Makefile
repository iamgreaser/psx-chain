CROSSPREFIX=mipsel-none-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs

ASFLAGS = -g -G0

CFLAGS = -g -c \
	-O2 -fomit-frame-pointer -fno-stack-protector -G0 -flto \
	-msoft-float -nostdlib -mips1 -march=3000 \
	-Isrc -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

LDFLAGS = -g -Wl,-T,link.ld -flto \
	-msoft-float \
	-L/usr/local/mipsel-none-elf/lib/soft-float/ \
	-L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ \
	-lm -lc -lgcc -lnullmon

EXE_NAME=boot
ISO_NAME=chaintest

OBJDIR = obj
SRCDIR = src
INCLUDES = src/psx.h src/common.h src/GL/gl.h src/GL/intern.h

OBJS = $(OBJDIR)/head.o \
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

$(EXE_NAME).exe: $(OBJS)
	$(CROSS_OBJCOPY) -O binary obj/$(EXE_NAME).elf $(EXE_NAME).exe

obj/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o obj/$(EXE_NAME).elf $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/head.o: $(SRCDIR)/head.S $(INCLUDES)
	$(CROSS_AS) -o $@ $(ASFLAGS) $<

