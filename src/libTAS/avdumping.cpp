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

#include "avdumping.h"
#ifndef LIBTAS_DISABLE_AVDUMPING

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include "hook.h"
#include "logging.h"
#include "videocapture.h"

FILE *f;
std::string filename;
AVFrame *frame;
AVPacket pkt;
AVCodecContext *c= NULL;
struct SwsContext *toYUVctx = NULL;
AVOutputFormat *outputFormat = NULL;
AVFormatContext *formatContext = NULL;
AVStream* video_st;

int openAVDumping(void* window, int video_opengl, std::string dumpfile) {

    int width, height;
    initVideoCapture(window, video_opengl, &width, &height);

    filename = dumpfile;

    /* Initialize AVCodec and AVFormat libraries */
    av_register_all();

    /* Initialize AVOutputFormat */
    outputFormat = av_guess_format(NULL, filename.c_str(), NULL);
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
    AVCodecID codec_id = AV_CODEC_ID_MPEG4;
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
    av_dump_format(formatContext, 0, filename.c_str(), 1);
    
    /* Set up output file */
    if (avio_open(&formatContext->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
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

    const uint8_t* orig_plane[4] = {0};
    int orig_stride[4] = {0};

    /* Access to the screen pixels */
    captureVideoFrame(window, orig_plane, orig_stride);

    /* Initialize AVPacket */
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;


    /* Change pixel format to YUV420p and copy it into the AVframe */
    int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
                frame->height, frame->data, frame->linesize);
    if (rets != frame->height) {
        debuglog(LCF_DUMP | LCF_ERROR, "We could only convert ",rets," rows");
        return 1;
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


int closeAVDumping() {
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

    /* Write file trailer */
    av_write_trailer(formatContext);

    /* Free resources */
    avio_close(formatContext->pb);
    avcodec_close(video_st->codec);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);

    return 0;
}

#endif

