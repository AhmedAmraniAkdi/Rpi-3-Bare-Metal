CFLAGS = -Wall -Werror -g  -Og -Ilibunix  -Wno-unused-function  -Wall
CC = gcc
SRC = my-install.c simple-boot.c  send-recv.c
OBJS = $(SRC:.c=.o)

all : libunix my-install # run

libunix:
	make -C libunix

my-install:  $(OBJS)  libunix/libunix.a
	$(CC) $(CFLAGS) $(OBJS) -o my-install -lm libunix/libunix.a

clean :
	rm -f *.o *~ Makefile.bak my-install blinker.bin tags
	make -C libunix clean

.PHONY: run clean libunix