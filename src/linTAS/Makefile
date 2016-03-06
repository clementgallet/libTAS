CC=gcc
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
#INCL=-L/home/clement/libTAS/src/external/criu/lib/c -Wl,-rpath=/home/clement/libTAS/src/external/criu/lib/c
INCL=
CFLAGS=-std=gnu11 -c -g
#CFLAGS=-std=gnu11 -c -g -m32
LDFLAGS=-lX11
#LDFLAGS=-lX11 -m32
SRCS=$(wildcard *.c) $(wildcard ../shared/*.c)
OBJS=$(SRCS:.c=.o)
DEPS=

.PHONY: all lintas clean

all: lintas

lintas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(INCL) -o ../../bin/linTAS

%.o: %.c $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS)  $< -o $@

clean:
	rm $(OBJS)

