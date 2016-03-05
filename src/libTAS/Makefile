CC=gcc
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
CFLAGS=-std=gnu99 -fpic -c -g
#CFLAGS=-std=gnu99 -fpic -c -g -m32
LDFLAGS=-shared -lX11 -lavcodec -lswscale -lavformat -lfreetype
#LDFLAGS=-shared -lX11 -m32
SRCS=$(wildcard *.c) $(wildcard ../shared/*.c) $(wildcard ../external/*.c)
OBJS=$(SRCS:.c=.o)
DEPS=../external/SDL.h
INCL=-I/usr/include/freetype2

.PHONY: all libtas clean
	
all: libtas

libtas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o ../../bin/libTAS.so

%.o: %.c $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS) $(INCL) $< -o $@

clean:
	rm $(OBJS)

