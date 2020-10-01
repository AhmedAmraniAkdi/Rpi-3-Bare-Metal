CC= aarch64-none-elf-gcc

LIBPI = ../libpi-WIP
CFLAGS = -I$(LIBPI) -I. -Wall  -Wno-unused-function -O2 -nostdlib -nostartfiles -ffreestanding 

SRC = blinker.c 
OBJS = $(SRC:.c=.o)

all : kernel8.img

_start.o : _start.s
	aarch64-none-elf-as _start.s -o _start.o

kernel8.img : mmap_blinker _start.o $(OBJS) $(LIBPI)/libpi.a
	aarch64-none-elf-ld _start.o $(OBJS) $(LIBPI)/libpi.a -T mmap_blinker -o blinker.elf
	aarch64-none-elf-objdump -D blinker.elf > blinker.list
	aarch64-none-elf-objcopy blinker.elf -O binary blinker.bin
	cp blinker.bin ../../unix_side-WIP/

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak

depend:
	makedepend -I$(LIBPI) *.[ch] 

.PHONY: clean depend all


bootloader.o: $(LIBPI)/rpi.h $(LIBPI)/timer.h $(LIBPI)/gpio.h