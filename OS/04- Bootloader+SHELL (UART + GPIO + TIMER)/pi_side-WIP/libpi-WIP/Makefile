LIB_OBJS = reboot.o timer.o helpers.o memmorybarrier.o
EXTRA= gpio.o mini_uart.o
SRC = $(LIB_OBJS:.o=.c)

ARM = aarch64-none-elf
CC = $(ARM)-gcc
LD  = $(ARM)-ld
AS  = $(ARM)-as
OD  = $(ARM)-objdump
OCP = $(ARM)-objcopy
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding  -march=armv6 -Og -std=gnu99

TARGET = libpi.a

all : $(TARGET) 

.s.o:
	$(AS) $(ASFLAGS) $< -o $@ 

.c.o:
	$(CC) $(CFLAGS) -c $< 

$(TARGET): $(LIB_OBJS) 
	arm-none-eabi-ar crf libpi.a $^ $(EXTRA)
	$(OD) -D $@ > $@.list

clean :
	rm -f $(LIB_OBJS) start.o *.bin *.hex *.elf *.list *.img *.bc *.clang.s *~ $(TARGET)  Makefile.bak

depend: 
	makedepend $(SRC)

reboot.o: rpi.h reboot.h
timer.o: rpi.h timer.h
mini_uart.o: gpio.h rpi.h