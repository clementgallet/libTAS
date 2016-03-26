CC=g++
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
CFLAGS=-std=gnu++11 -fpic -c -g -fvisibility=hidden -fvisibility-inlines-hidden $(TARCH)
#CFLAGS=-std=gnu99 -fpic -c -g -m32
#LDFLAGS=-shared -lX11 -lavcodec -lswscale -lavformat -lfreetype
LDFLAGS=-shared -lX11 $(TARCH) -lsndfile -lavresample
SRCS=$(wildcard *.cpp) $(wildcard audio/*.cpp) $(wildcard ../shared/*.cpp) $(wildcard ../external/*.cpp)
OBJS=$(SRCS:.cpp=.o)
DEPS=../external/SDL.h
INCL=-I/usr/include/freetype2

.PHONY: all libtas clean
	
all: libtas

libtas: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o ../../bin/libTAS.so

%.o: %.cpp $(DEPS)
	$(CC) $(WARNINGS) $(CFLAGS) $(INCL) $< -o $@

