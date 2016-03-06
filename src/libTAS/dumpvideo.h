#ifndef DUMPVIDEO_H_INCL
#define DUMPVIDEO_H_INCL

#ifdef LIBTAS_DUMP


int openVideoDump(void* window, int video_opengl, char* filename);
int encodeOneFrame(unsigned long fcounter, void* window);
int closeVideoDump();

#endif
#endif
