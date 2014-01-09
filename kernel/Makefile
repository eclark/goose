# Makefile based on 391 but modified for 64 bit higher-half.

CFLAGS += -std=gnu99 -Wall -m64 -mcmodel=large -ffreestanding -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
CPPFLAGS += -Iinclude
ASFLAGS += -m64
LDFLAGS += -nostdlib -static -z max-page-size=0x1000
CC=cross-gcc
NM=cross-nm

CPPFLAGS += -ffreestanding -g

SRC = $(wildcard *.S) $(wildcard *.c) $(wildcard **/*.S) $(wildcard **/*.c)

OBJS  = system/entry.o
OBJS += $(filter-out system/entry.o,$(patsubst %.S,%.o,$(filter %.S,$(SRC))))
OBJS += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

os.iso: ../iso/boot/kernel.sys
	grub-mkrescue -d /usr/lib/grub/i386-pc -o $@ ../iso

../iso/boot/kernel.sys: kernel.sys kernel.sym
	cp $< $@

kernel.sys: Makefile linker.ld $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -Tlinker.ld -o $@

kernel.sym: kernel.sys
	$(NM) --defined $< | cut -f 1,3 -d ' ' >$@

Makefile.dep: $(SRC)
	$(CC) -M $(CPPFLAGS) $(SRC) >$@

.PHONY: clean
clean:
	rm -f *.o */*.o Makefile.dep kernel.sys kernel.sym os.iso

ifneq ($(MAKECMDGOALS),dep)
ifneq ($(MAKECMDGOALS),clean)
include Makefile.dep
endif
endif
