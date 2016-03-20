CC=g++
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
CFLAGS=-std=gnu++11 -c -g $(TARCH)
LDFLAGS=-lX11 $(TARCH)
SRCS=$(wildcard *.cpp) $(wildcard ../shared/*.cpp)
OBJS=$(SRCS:.cpp=.o)
DEPS=

.PHONY: all lintas clean

all: lintas

lintas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o ../../bin/linTAS

%.o: %.cpp $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS)  $< -o $@

