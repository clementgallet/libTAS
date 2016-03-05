CC=gcc
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
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
	$(CC) $(LDFLAGS) $(OBJS) -o ../../bin/linTAS

%.o: %.c $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)

