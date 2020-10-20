# change this
# TODO change change chaaaange this to work with new structure
LIB_OBJS = reboot.o timer.o helpers.o memorybarrier.o gpio.o mini_uart.o
SRC = $(LIB_OBJS:.o=.c)

ARM = aarch64-none-elf
CC = $(ARM)-gcc
LD  = $(ARM)-ld
AS  = $(ARM)-as
OD  = $(ARM)-objdump
OCP = $(ARM)-objcopy
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -O2 -std=gnu99

TARGET = libpi.a

all : $(TARGET) 

%.o:%.s
	$(AS) $(ASFLAGS) $< -o $@ 

%.o:%.c
	$(CC) $(CFLAGS) -c $< 

$(TARGET): $(LIB_OBJS) 
	aarch64-none-elf-ar crf libpi.a $^
	$(OD) -D $@ > $@.list

clean :
	rm -f $(LIB_OBJS) start.o *.bin *.hex *.elf *.list *.img *.bc *.clang.s *~ $(TARGET)  Makefile.bak

depend: 
	makedepend $(SRC)

reboot.o: rpi.h
timer.o: rpi.h timer.h
mini_uart.o: gpio.h rpi.h
