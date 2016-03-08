.PHONY: all clean src/libTAS src/linTAS 

all: 64bit

64bit:
	$(MAKE) -C src/libTAS/ TARCH=-m64
	$(MAKE) -C src/linTAS/ TARCH=-m64

32bit:
	$(MAKE) -C src/libTAS/ TARCH=-m32
	$(MAKE) -C src/linTAS/ TARCH=-m32

clean:
	$(MAKE) -C src/libTAS/ clean
	$(MAKE) -C src/linTAS/ clean

