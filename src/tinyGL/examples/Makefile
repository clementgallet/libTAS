include ../makeinclude

PROGS = gears triangle

all: $(PROGS)

clean:
	rm -f core *.o *~ $(PROGS)

gears: gears.o
	$(CC) $(LFLAGS) $^ -o $@ $(GL_LIBS) $(UI_LIBS) -lm

triangle: triangle.o
	$(CC) $(LFLAGS) $^ -o $@ -lTinyGLU $(GL_LIBS) $(UI_LIBS) -lm

.c.o:
	$(CC) $(CFLAGS) $(GL_INCLUDES) $(UI_INCLUDES) -c $*.c
