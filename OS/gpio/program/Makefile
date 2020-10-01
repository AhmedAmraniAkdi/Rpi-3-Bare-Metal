CC= aarch64-none-elf-gcc

CFLAGS = -I. -Wall  -Wno-unused-function -O2 -nostdlib -nostartfiles -ffreestanding 

SRC = blinker.c reboot.c timer.c gpio.c
OBJS = $(SRC:.c=.o)

all : kernel8.img

%.o:%.s
	aarch64-none-elf-as $< -o $@ 

kernel8.img : mmap_blinker _start.o helpers.o $(OBJS)
	aarch64-none-elf-ld _start.o $(OBJS) helpers.o -T mmap_blinker -o blinker.elf
	aarch64-none-elf-objdump -D blinker.elf > blinker.list
	aarch64-none-elf-objcopy blinker.elf -O binary kernel8.img

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak

depend:
	makedepend -I$(LIBPI) *.[ch] 

.PHONY: clean depend all

kernel8.img: rpi.h timer.h gpio.h