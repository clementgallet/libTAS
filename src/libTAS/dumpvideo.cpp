/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dumpvideo.h"
#ifdef LIBTAS_DUMP

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include "hook.h"
#include "logging.h"

FILE *f;
char* filename = NULL;
AVFrame *frame;
AVPacket pkt;
AVCodecContext *c= NULL;
struct SwsContext *toYUVctx = NULL;
AVOutputFormat *outputFormat = NULL;
AVFormatContext *formatContext = NULL;
AVStream* video_st;

uint8_t* glpixels;
uint8_t* glpixels_flip;

int useGL;

/* Original function pointers */
void (*SDL_GL_GetDrawableSize_real)(void* window, int* w, int* h);
void* (*SDL_GetWindowSurface_real)(void* window);
int (*SDL_LockSurface_real)(void* surface);
void (*SDL_UnlockSurface_real)(void* surface);
void (*glReadPixels_real)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);

int openVideoDump(void* window, int video_opengl, char* dumpfile) {

    /* FIXME: Does not work with SDL 1.2 !!! */
    LINK_SUFFIX(SDL_GL_GetDrawableSize, "libSDL2-2");
    LINK_SUFFIX(SDL_GetWindowSurface, "libSDL2-2");
    LINK_SUFFIX(SDL_LockSurface, "libSDL2-2");
    LINK_SUFFIX(SDL_UnlockSurface, "libSDL2-2");
    LINK_SUFFIX(glReadPixels, "libGL");

    filename = dumpfile;

    /* Get information about the current screen */
    int width, height;
    SDL_GL_GetDrawableSize_real(window, &width, &height);

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglog(LCF_DUMP | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return 1;
    }

    /* If the game uses openGL, the method to capture the screen will be different */
    useGL = video_opengl;

    if (useGL) {

        /* Do we already have access to the glReadPixels function? */
        if (!glReadPixels_real) {
            debuglog(LCF_DUMP | LCF_OGL | LCF_ERROR, "Could not load function glReadPixels.");
            return 1;
        }

        /* Initialize buffers for screen pixels */
        int size = width * height * 4;
        glpixels = malloc(size);
        glpixels_flip = malloc(size);
    }

    /* Initialize AVCodec and AVFormat libraries */
    av_register_all();

    /* Initialize AVOutputFormat */
    
    outputFormat = av_guess_format(NULL, filename, NULL);
    if (!outputFormat) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not find suitable output format");
        return 1;
    }

    /* Initialize AVFormatContext */

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize AVFormatContext");
        return 1;
    }
    formatContext->oformat = outputFormat;

    /* Initialize AVCodec */

    AVCodec *codec = NULL;
    int codec_id = AV_CODEC_ID_MPEG4;
    //int codec_id = AV_CODEC_ID_H264;
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Codec not found");
        return 1;
    }
    outputFormat->video_codec = codec_id;

    /* Initialize video stream */

    video_st = avformat_new_stream(formatContext, codec);
    if (!video_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        return 1;
    }

    /* Fill video stream parameters */
    video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_st->codec->codec_id = codec_id;

    video_st->codec->bit_rate = 400000;
    video_st->codec->width = width;
    video_st->codec->height = height;
    video_st->time_base = (AVRational){1,60}; // TODO: Put the actual framerate
    video_st->codec->time_base = (AVRational){1,60};
    video_st->codec->gop_size = 10; /* emit one intra frame every ten frames */
    video_st->codec->max_b_frames = 1;
    video_st->codec->pix_fmt = AV_PIX_FMT_YUV420P;

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Use a preset for h264 */
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_st->codec->priv_data, "preset", "slow", 0);

    /* Open the codec */
    if (avcodec_open2(video_st->codec, codec, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open codec");
        return 1;
    }
    
   
    /* Initialize AVFrame */

    frame = av_frame_alloc();
    if (!frame) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate AVFrame");
        return 1;
    }
    frame->format = video_st->codec->pix_fmt;
    frame->width  = video_st->codec->width;
    frame->height = video_st->codec->height;

    /* Allocate the image buffer inside the AVFrame */

    int ret = av_image_alloc(frame->data, frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate raw picture buffer");
        return 1;
    }


    /* Initialize swscale context for pixel format conversion */

    toYUVctx = sws_getContext(frame->width, frame->height,  
                              PIX_FMT_RGBA,
                              frame->width, frame->height, 
                              PIX_FMT_YUV420P,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    if (toYUVctx == NULL) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate swscale context");
        return 1;
    }

    /* Print informations on input and output streams */
    av_dump_format(formatContext, 0, filename, 1);
    
    /* Set up output file */
    if (avio_open(&formatContext->pb, filename, AVIO_FLAG_WRITE) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video file");
        return 1;
    }

    /* Write header */
    if (avformat_write_header(formatContext, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not write header");
        return 1;
    }

    return 0;
}

/*
 * Encode one video frame and send it to the muxer
 * Returns 0 if no error was encountered
 */

int encodeOneFrame(unsigned long fcounter, void* window) {

    debuglog(LCF_DUMP | LCF_FRAME, "Encode a video frame");

    /* Initialize AVPacket */
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;

    SDL_Surface* surface = NULL;

    const uint8_t* orig_plane[4] = {0};
    int orig_stride[4] = {0};

    if (useGL) {
        /* TODO: Check that the openGL dimensions did not change in between */

        int size = frame->width * frame->height * 4;
        /* We access to the image pixels directly using glReadPixels */
        glReadPixels_real(0, 0, frame->width, frame->height, /* GL_RGBA */ 0x1908, /* GL_UNSIGNED_BYTE */ 0x1401, glpixels);
        /* TODO: I saw this in some examples before calling glReadPixels: glPixelStorei(GL_PACK_ALIGNMENT, 1); */

        /*
         * Flip image horizontally
         * This is because OpenGL has a different reference point
         * Code taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
         * TODO: Could this be done without allocating another array ?
         */

        int pitch = 4 * frame->width;
        for (int line = 0; line < frame->height; line++) {
            int pos = line * pitch;
            for (int p=0; p<pitch; p++) {
                glpixels_flip[pos + p] = glpixels[(size-pos)-pitch + p];
            }
        }
        
        orig_plane[0] = (const uint8_t*)(glpixels_flip);
        orig_stride[0] = frame->width * 4;

    }

    else {
        /* Not tested !! */
        debuglog(LCF_DUMP | LCF_UNTESTED | LCF_FRAME, "Access SDL_Surface pixels for video dump");

        /* Get surface from window */
        surface = SDL_GetWindowSurface_real(window);

        /* Currently only supporting ARGB (32 bpp) pixel format */
        if (surface->format->BitsPerPixel != 32) {
            debuglog(LCF_DUMP | LCF_ERROR, "Bad bpp for surface: %d\n", surface->format->BitsPerPixel);
            return 1;
        }

        /* Checking for a size modification */
        if ((surface->w != frame->width) || (surface->h != frame->height)) {
            debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",frame->width,",",frame->height,") -> (",surface->w,",",surface->h,")");
            return 1;
        }

        /* We must lock the surface before accessing the raw pixels */
        if (0 != SDL_LockSurface_real(surface)) {
            debuglog(LCF_DUMP | LCF_ERROR, "Could not lock SDL surface");
            return 1;
        }

        orig_plane[0] = (const uint8_t*)(surface->pixels);
        orig_stride[0] = surface->pitch;
    }


    /* Change pixel format to YUV420p and copy it into the AVframe */
    int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
                frame->height, frame->data, frame->linesize);
    if (rets != frame->height) {
        debuglog(LCF_DUMP | LCF_ERROR, "We could only convert ",rets," rows");
        if (!useGL) {
            /* Unlock surface */
            SDL_UnlockSurface_real(surface);
        }
        return 1;
    }

    if (!useGL) {
        /* Unlock surface */
        SDL_UnlockSurface_real(surface);
    }

    frame->pts = fcounter;

    /* Encode the image */
    int got_output;
    int ret = avcodec_encode_video2(video_st->codec, &pkt, frame, &got_output);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Error encoding frame");
        return 1;
    }

    if (got_output) {
        /* We have an encoder output to write */
        pkt.pts = fcounter; // TODO: check this value
        pkt.dts = fcounter; // TODO: check this value
        if (av_write_frame(formatContext, &pkt) < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
            return 1;
        }
        debuglog(LCF_DUMP | LCF_FRAME, "Write frame ",fcounter," (size=",pkt.size,")");
        av_free_packet(&pkt);
    }

    return 0;
}


int closeVideoDump() {
    /* Encode the remaining frames */
    for (int got_output = 1; got_output;) {
        int ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error encoding frame");
            return 1;
        }

        if (got_output) {
            if (av_write_frame(formatContext, &pkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return 1;
            }
            debuglog(LCF_DUMP | LCF_FRAME, "Write frame -1 (size=", pkt.size, ")");
            av_free_packet(&pkt);
        }
    }

    if (useGL) {
        free(glpixels);
        free(glpixels_flip);
    }

    /* Write file trailer */
    av_write_trailer(formatContext);

    /* Free resources */
    avio_close(formatContext->pb);
    avcodec_close(video_st->codec);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    free(filename);

    return 0;
}

#endif
