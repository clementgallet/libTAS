include makeinclude

all:
	( for f in $(DIRS); do ( cd $$f ; make all ) || exit 1 ; done )

clean:
	rm -f *~ lib/libTinySDGL.a lib/libTinyGLU.a include/GL/*~ TAGS
	( for f in $(DIRS); do ( cd $$f ; make clean ; ) done )

install:
	( for f in $(DIRS); do ( cd $$f ; make install ; ) done )
