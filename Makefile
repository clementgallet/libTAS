.PHONY: all clean src/libTAS src/linTAS 

all:
	$(MAKE) -C src/libTAS/
	$(MAKE) -C src/linTAS/

clean:
	$(MAKE) -C src/libTAS/ clean
	$(MAKE) -C src/linTAS/ clean

