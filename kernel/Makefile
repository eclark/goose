# Makefile based on 391 but modified for 64 bit higher-half.

CFLAGS += -Wall -m64 -mcmodel=large -ffreestanding -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
ASFLAGS +=
LDFLAGS += -nostdlib -static -z max-page-size=0x1000
CC=cross-gcc
NM=cross-nm

CPPFLAGS += -nostdinc -g

SRC = $(wildcard *.S) $(wildcard *.c)

OBJS  = entry.o
OBJS += $(filter-out entry.o,$(patsubst %.S,%.o,$(filter %.S,$(SRC))))
OBJS += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

os.iso: iso/boot/kernel.sys
	grub-mkrescue -d /usr/lib/grub/i386-pc -o $@ iso

iso/boot/kernel.sys: kernel.sys kernel.sym
	cp $< $@

kernel.sys: Makefile linker.ld $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -Tlinker.ld -o $@

kernel.sym: kernel.sys
	$(NM) --defined $< | cut -f 1,3 -d ' ' >$@

Makefile.dep: $(SRC)
	$(CC) -MM $(CPPFLAGS) $(SRC) >$@

.PHONY: clean
clean:
	rm -f *.o Makefile.dep kernel.sys

ifneq ($(MAKECMDGOALS),dep)
ifneq ($(MAKECMDGOALS),clean)
include Makefile.dep
endif
endif