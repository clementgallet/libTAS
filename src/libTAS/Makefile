CC=gcc
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
CFLAGS=-std=c99 -fpic -c -g
LDFLAGS=-shared -lX11 -lavcodec -lswscale -lavformat
SRCS=$(wildcard *.c) $(wildcard ../shared/*.c)
OBJS=$(SRCS:.c=.o)
DEPS=SDL.h

.PHONY: all libtas clean
	
all: libtas

libtas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o ../../bin/libTAS.so

%.o: %.c $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)

