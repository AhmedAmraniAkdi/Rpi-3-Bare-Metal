SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.s)
LIB_OBJS =  $(patsubst %.c, %.o, $(SRC)) $(patsubst %.s, %.o, $(ASM_SRC))

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
	rm -f $(LIB_OBJS) *.bin *.hex *.elf *.list *.img *.bc *.clang.s *~ $(TARGET)  Makefile.bak

depend: 
	makedepend $(SRC)

