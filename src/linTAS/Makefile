CC=g++
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
#INCL=-L/home/clement/libTAS/src/external/criu/lib/c -Wl,-rpath=/home/clement/libTAS/src/external/criu/lib/c
INCL=
CFLAGS=-std=gnu++11 -c -g $(TARCH)
LDFLAGS=-lX11 $(TARCH)
SRCS=$(wildcard *.cpp) $(wildcard ../shared/*.cpp)
OBJS=$(SRCS:.cpp=.o)
DEPS=

.PHONY: all lintas clean

all: lintas

lintas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(INCL) -o ../../bin/linTAS

%.o: %.cpp $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS)  $< -o $@

