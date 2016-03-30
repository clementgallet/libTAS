.PHONY: all clean src/libTAS src/linTAS 
OBJS=$(wildcard src/libTAS/*.o) $(wildcard src/shared/*.o) $(wildcard src/linTAS/*.o) $(wildcard src/external/*.o) 

#DEFS=-DLIBTAS_DISABLE_AVDUMPING
DEFS=

all: 64bit

64bit:
	$(MAKE) -C src/libTAS/ TARCH=-m64 DEFS=$(DEFS)
	$(MAKE) -C src/linTAS/ TARCH=-m64 DEFS=$(DEFS)

32bit:
	$(MAKE) -C src/libTAS/ TARCH=-m32 DEFS=$(DEFS)
	$(MAKE) -C src/linTAS/ TARCH=-m32 DEFS=$(DEFS)

clean:
	rm $(OBJS)

