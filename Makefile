CROSSPREFIX=mipsel-none-elf-

CC=$(CROSSPREFIX)gcc
AS=$(CROSSPREFIX)as
OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f

ASFLAGS = -g -G0

CFLAGS = -g -c \
	-O2 -fomit-frame-pointer -fno-stack-protector -G0 -flto \
	-msoft-float -nostdlib -mips1 -march=3000 \
	-Isrc -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

LDFLAGS = -g -Wl,-T,link.ld \
	-msoft-float \
	-L/usr/local/mipsel-none-elf/lib/soft-float/ \
	-L/usr/local/lib/gcc/mipsel-none-elf/4.7.0/soft-float/ \
	-lm -lc -lgcc -lnullmon

EXE_NAME=boot
ISO_NAME=chaintest

OBJDIR = obj
SRCDIR = src
INCLUDES = src/psx.h src/GL/gl.h

OBJS = $(OBJDIR)/head.o \
	\
	$(OBJDIR)/main.o

all: $(EXE_NAME).exe

clean:
	$(RM_F) $(OBJS)

$(EXE_NAME).exe: $(OBJS)
	$(OBJCOPY) -O binary obj/$(EXE_NAME).elf $(EXE_NAME).exe

obj/$(EXE_NAME).elf: $(OBJS)
	$(CC) -o obj/$(EXE_NAME).elf $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/head.o: $(SRCDIR)/head.S $(INCLUDES)
	$(AS) -o $@ $(ASFLAGS) $<

