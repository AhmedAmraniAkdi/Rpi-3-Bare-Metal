CC= aarch64-none-elf-gcc

LIBPI = libpi
CFLAGS = -I$(LIBPI) -I. -Wall  -Wno-unused-function -O2 -nostdlib -nostartfiles -ffreestanding 

#SRC = testing_armtimer_int.c 
#SRC = test-kmalloc.c
#SRC = test_genericlinkedlists.c
SRC = thread_test_mmu_turbo_multi01.c
#SRC = testing_vcmb.c
#SRC = testing_fpu.c
OBJS = $(SRC:.c=.o)

all : libpi kernel8.img 

libpi:
	make -C $(LIBPI)

start.o : start.s
	aarch64-none-elf-as start.s -o start.o
	#$(CC) -c $< -o $@ #for start.S

kernel8.img : mmap start.o $(OBJS) $(LIBPI)/libpi.a
	aarch64-none-elf-ld start.o $(OBJS) $(LIBPI)/libpi.a -T mmap -o test.elf
	aarch64-none-elf-objdump -D test.elf > test.list
	aarch64-none-elf-objcopy test.elf -O binary kernel8.img

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak
	make -C $(LIBPI) clean

depend:
	makedepend -I$(LIBPI) *.[ch] 

.PHONY: clean depend libpi