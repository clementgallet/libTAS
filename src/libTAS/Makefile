CC=g++
WARNINGS=-Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow
CFLAGS=-std=gnu++11 -fpic -c -g -fvisibility=hidden -fvisibility-inlines-hidden $(TARCH) $(DEFS)
CFLAGS+= `pkg-config --cflags libavresample libavcodec libswscale libavformat libavutil libpulse-simple`
#LDFLAGS=-shared -lX11 -lfreetype
#LDFLAGS=-shared -lX11 $(TARCH) -lsndfile -lavresample -lavcodec -lswscale -lavformat -lpulse-simple
LDFLAGS=-shared -lX11 $(TARCH)
LDFLAGS+= `pkg-config --libs libavresample libavcodec libswscale libavformat libavutil libpulse-simple`
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

