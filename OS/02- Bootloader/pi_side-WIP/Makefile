# the other makefile
# https://github.com/dddrrreee/cs140e-20win/blob/master/libpi/Makefile
# is more general, will use this one for now,
# the other is the same as the one used on libunix
# will use it when there is more stuff

CC= aarch64-none-elf-gcc

LIBPI = libpi-WIP
CFLAGS = -I$(LIBPI) -I. -Wall  -Wno-unused-function -O2 -nostdlib -nostartfiles -ffreestanding 

SRC = bootloader.c 
OBJS = $(SRC:.c=.o)

# Uncomment copy and fix up where your SD card is.
all : libpi kernel8.img # copy

libpi:
	make -C $(LIBPI)
	make -C ./program

start.o : start.s
	aarch64-none-elf-as start.s -o start.o

kernel8.img : mmap start.o $(OBJS)  $(LIBPI)/libpi.a
	aarch64-none-elf-ld start.o $(OBJS) $(LIBPI)/libpi.a -T mmap -o bootloader.elf
	aarch64-none-elf-objdump -D bootloader.elf > bootloader.list
	aarch64-none-elf-objcopy bootloader.elf -O binary kernel8.img

# change this to copy whereever your sd is mounted.
copy:
	cp kernel.img /media/engler/0403-0201/
	sync

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak
	make -C $(LIBPI) clean
	make -C ./program clean

depend:
	makedepend -I$(LIBPI) *.[ch] 

.PHONY: clean depend copy libpi


bootloader.o: ../shared-code/simple-boot.h $(LIBPI)/rpi.h $(LIBPI)/mini_uart.h $(LIBPI)/timer.h $(LIBPI)/gpio.h