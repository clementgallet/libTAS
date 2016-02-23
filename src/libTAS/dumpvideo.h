#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include "hook.h"

#define RNDTO2(X) ( (X) & 0xFFFFFFFE )
#define RNDTO32(X) ( ( (X) % 32 ) ? ( ( (X) + 32 ) & 0xFFFFFFE0 ) : (X) )

void openVideoDump(void* window);
void encodeOneFrame(unsigned long fcounter, void* window);
void closeVideoDump();
