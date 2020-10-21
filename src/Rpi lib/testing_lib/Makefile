TEST = test-kmalloc.c

SRC = $(wildcard *.c)
SRC = $(filter-out $(TEST), $(SRC))

ASM_SRC = $(wildcard *.s)

LIB_OBJS =  $(patsubst %.c, %.o, $(SRC)) $(patsubst %.s, %.o, $(ASM_SRC))

ARM = aarch64-none-elf
CC = $(ARM)-gcc
LD  = $(ARM)-ld
AS  = $(ARM)-as
OD  = $(ARM)-objdump
OCP = $(ARM)-objcopy
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -O2 -std=gnu99

all : $(TEST) libpi.a kernel8.img

%.o:%.s
	$(AS) $(ASFLAGS) $< -o $@ 

%.o:%.c
	$(CC) $(CFLAGS) -c $< 

libpi.a: $(LIB_OBJS) 
	aarch64-none-elf-ar crf libpi.a $^
	$(OD) -D $@ > $@.list

kernel8.img : mmap start.o $(TEST) libpi.a
	aarch64-none-elf-ld start.o $(TEST) libpi.a -T mmap -o test_kernel.elf
	aarch64-none-elf-objdump -D test_kernel.elf > test_kernel.list
	aarch64-none-elf-objcopy test_kernel.elf -O binary kernel8.img

clean :
	rm -f $(LIB_OBJS) start.o *.bin *.hex *.elf *.list *.img *.bc *.clang.s *~ *.a  Makefile.bak

depend: 
	makedepend $(SRC)

.PHONY: clean depend libpi

