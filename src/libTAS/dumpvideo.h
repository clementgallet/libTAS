#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include "hook.h"

void openVideoDump(void* window);
void encodeOneFrame(unsigned long fcounter);
void closeVideoDump();
