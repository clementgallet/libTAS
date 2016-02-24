#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include "hook.h"
#include "logging.h"

#define RNDTO2(X) ( (X) & 0xFFFFFFFE )

int openVideoDump(void* window, int video_opengl, char* filename);
int encodeOneFrame(unsigned long fcounter, void* window);
int closeVideoDump();
