.PHONY: all clean src/libTAS src/linTAS 
OBJS=$(wildcard src/libTAS/*.o) $(wildcard src/shared/*.o) $(wildcard src/linTAS/*.o) $(wildcard src/external/*.o) 

all: 64bit

64bit:
	$(MAKE) -C src/libTAS/ TARCH=-m64
	$(MAKE) -C src/linTAS/ TARCH=-m64

32bit:
	$(MAKE) -C src/libTAS/ TARCH=-m32
	$(MAKE) -C src/linTAS/ TARCH=-m32

clean:
	rm $(OBJS)

