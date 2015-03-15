include ../makeinclude

OBJS= api.o list.o vertex.o init.o matrix.o texture.o \
      misc.o clear.o light.o clip.o select.o get.o error.o \
      zbuffer.o zline.o zdither.o ztriangle.o \
      zmath.o image_util.o oscontext.o msghandling.o \
      arrays.o specbuf.o memory.o

INCLUDES = -I../include
LIB = libTinySDGL.a

all: $(LIB) libTinyGLU.a

$(LIB): $(OBJS)
	rm -f $(LIB)
	ar rcs $(LIB) $(OBJS)
	mv $(LIB) ../lib

libTinyGLU.a: glu.o
	rm -f $@
	ar rcs $@ $<
	mv $@ ../lib

clean:
	rm -f *~ *.o *.a

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clip.o: zgl.h ../include/zfeatures.h
vertex.o: zgl.h ../include/zfeatures.h
light.o: zgl.h ../include/zfeatures.h
matrix.o: zgl.h ../include/zfeatures.h
list.o: zgl.h opinfo.h ../include/zfeatures.h
arrays.c: zgl.h ../include/zfeatures.h
specbuf.o: zgl.h ../include/zfeatures.h
zline.o: zgl.h ../include/zfeatures.h zline.h
glu.o: glu.c

ztriangle.o: ztriangle.c ztriangle.h zgl.h ../include/zfeatures.h
	$(CC) $(CFLAGS) -Wno-uninitialized $(INCLUDES) -c $*.c
