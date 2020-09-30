CFLAGS = -Wall -Werror -g  -Og -Ilibunix  -Wno-unused-function  -Wall
CC = gcc
SRC = my-install.c simple-boot.c  send-recv.c
OBJS = $(SRC:.c=.o)

all : libunix my-install # run

libunix:
	make -C libunix

my-install:  $(OBJS)  libunix/libunix.a
	$(CC) $(CFLAGS) $(OBJS) -o my-install -lm libunix/libunix.a


run:
	./my-install ./testerino


clean :
	rm -f *.o *~ Makefile.bak my-install  tags
	make -C libunix clean

.PHONY: run clean libunix