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
#include "audio/AudioContext.h"

FILE *f;
std::string filename;
AVFrame* video_frame;
AVFrame* audio_frame;
struct SwsContext *toYUVctx = NULL;
AVOutputFormat *outputFormat = NULL;
AVFormatContext *formatContext = NULL;
AVStream* video_st;
AVStream* audio_st;

/* We save the frame when the dumping begins */
int start_frame;

/* The accumulated number of audio samples */
uint64_t accum_samples;

int openAVDumping(void* window, int video_opengl, std::string dumpfile, int sf) {

    start_frame = sf;
    accum_samples = 0;

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

    /*** Create video stream ***/

    /* Initialize video AVCodec */

    AVCodec *video_codec = NULL;
    AVCodecID codec_id = AV_CODEC_ID_MPEG4;
    //int codec_id = AV_CODEC_ID_H264;
    video_codec = avcodec_find_encoder(codec_id);
    if (!video_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Video codec not found");
        return 1;
    }
    outputFormat->video_codec = codec_id;

    /* Initialize video stream */

    video_st = avformat_new_stream(formatContext, video_codec);
    if (!video_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        return 1;
    }

    /* Fill video stream parameters */
    video_st->id = formatContext->nb_streams - 1;
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
    if (avcodec_open2(video_st->codec, video_codec, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video codec");
        return 1;
    }
    
    /*** Create audio stream ***/

    /* Initialize audio AVCodec */

    AVCodec *audio_codec = NULL;
    AVCodecID audio_codec_id = AV_CODEC_ID_PCM_S16LE;
    //AVCodecID audio_codec_id = AV_CODEC_ID_VORBIS;
    audio_codec = avcodec_find_encoder(audio_codec_id);
    if (!audio_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Audio codec not found");
        return 1;
    }

    /* Initialize audio stream */

    audio_st = avformat_new_stream(formatContext, audio_codec);
    if (!audio_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        return 1;
    }

    /* Fill audio stream parameters */

    audio_st->id = formatContext->nb_streams - 1;
    audio_st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    if (audiocontext.outBitDepth == 8)
        audio_st->codec->sample_fmt = AV_SAMPLE_FMT_U8;
    else if (audiocontext.outBitDepth == 16)
        audio_st->codec->sample_fmt = AV_SAMPLE_FMT_S16;
    else {
        debuglog(LCF_DUMP | LCF_ERROR, "Unknown audio format");
        return 1;
    }
    audio_st->codec->bit_rate = 64000;
    audio_st->codec->sample_rate = audiocontext.outFrequency;
    audio_st->codec->channels = audiocontext.outNbChannels;

    /* Some formats want stream headers to be separate. */

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audio_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Open the codec */
    if (avcodec_open2(audio_st->codec, audio_codec, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open audio codec");
        return 1;
    }

    /* Initialize video AVFrame */

    video_frame = av_frame_alloc();
    if (!video_frame) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate AVFrame");
        return 1;
    }
    video_frame->format = video_st->codec->pix_fmt;
    video_frame->width  = video_st->codec->width;
    video_frame->height = video_st->codec->height;

    /* Initialize audio AVFrame */
    audio_frame = av_frame_alloc();

    /* Allocate the image buffer inside the AVFrame */

    int ret = av_image_alloc(video_frame->data, video_frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate raw picture buffer");
        return 1;
    }


    /* Initialize swscale context for pixel format conversion */

    toYUVctx = sws_getContext(video_frame->width, video_frame->height,  
                              PIX_FMT_RGBA,
                              video_frame->width, video_frame->height, 
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
 * Encode one frame and send it to the muxer
 * Returns 0 if no error was encountered
 */

int encodeOneFrame(unsigned long fcounter, void* window) {

    /*** Video ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode a video frame");

    const uint8_t* orig_plane[4] = {0};
    int orig_stride[4] = {0};

    /* Access to the screen pixels */
    captureVideoFrame(window, orig_plane, orig_stride);

    /* Initialize AVPacket */
    AVPacket vpkt;
    vpkt.data = NULL;
    vpkt.size = 0;
    av_init_packet(&vpkt);

    /* Change pixel format to YUV420p and copy it into the AVframe */
    int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
                video_frame->height, video_frame->data, video_frame->linesize);
    if (rets != video_frame->height) {
        debuglog(LCF_DUMP | LCF_ERROR, "We could only convert ",rets," rows");
        return 1;
    }

    video_frame->pts = fcounter - start_frame;

    /* Encode the image */
    int got_output;
    int ret = avcodec_encode_video2(video_st->codec, &vpkt, video_frame, &got_output);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Error encoding video frame");
        return 1;
    }

    if (got_output) {
        /* Rescale output packet timestamp values from codec to stream timebase */
        vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_st->codec->time_base, video_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_st->codec->time_base, video_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.duration = av_rescale_q(vpkt.duration, video_st->codec->time_base, video_st->time_base);
        vpkt.stream_index = video_st->index;
        if (av_interleaved_write_frame(formatContext, &vpkt) < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
            return 1;
        }
        av_free_packet(&vpkt);
    }

    /*** Audio ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode an audio frame");

    /* Initialize AVPacket */
    AVPacket apkt;
    apkt.data = NULL;
    apkt.size = 0;
    av_init_packet(&apkt);

    int frame_size;
    if (audio_st->codec->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        frame_size = audiocontext.outNbSamples;
    else {
        frame_size = audio_st->codec->frame_size;
        if (frame_size > audiocontext.outNbSamples) {
            debuglog(LCF_DUMP | LCF_FRAME | LCF_ERROR, "This is bad...");
            frame_size = audiocontext.outNbSamples;
        }
    }

    audio_frame->nb_samples = frame_size;
    audio_frame->pts = av_rescale_q(accum_samples, (AVRational){1, audio_st->codec->sample_rate}, audio_st->codec->time_base);
    accum_samples += frame_size;

    avcodec_fill_audio_frame(audio_frame, audio_st->codec->channels, audio_st->codec->sample_fmt,
                                             &audiocontext.outSamples[0], frame_size*audiocontext.outAlignSize, 1);

    ret = avcodec_encode_audio2(audio_st->codec, &apkt, audio_frame, &got_output);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Error encoding audio frame");
        return 1;
    }

    if (got_output) {
        /* We have an encoder output to write */
        apkt.pts = av_rescale_q_rnd(apkt.pts, audio_st->codec->time_base, audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        apkt.dts = av_rescale_q_rnd(apkt.dts, audio_st->codec->time_base, audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        apkt.duration = av_rescale_q(apkt.duration, audio_st->codec->time_base, audio_st->time_base);
        apkt.stream_index = audio_st->index;
        if (av_interleaved_write_frame(formatContext, &apkt) < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
            return 1;
        }
        av_free_packet(&apkt);
    }

    return 0;
}


int closeAVDumping() {
    /* Encode the remaining frames */
    int got_video = 1;
    int got_audio = 1;
    for (; got_video || got_audio;) {

        /* Initialize AVPacket */
        AVPacket vpkt;
        vpkt.data = NULL;
        vpkt.size = 0;
        av_init_packet(&vpkt);

        int ret = avcodec_encode_video2(video_st->codec, &vpkt, NULL, &got_video);
        if (ret < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error encoding frame");
            return 1;
        }

        if (got_video) {
            vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_st->codec->time_base, video_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_st->codec->time_base, video_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.duration = av_rescale_q(vpkt.duration, video_st->codec->time_base, video_st->time_base);
            vpkt.stream_index = video_st->index;
            if (av_interleaved_write_frame(formatContext, &vpkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return 1;
            }
            av_free_packet(&vpkt);
        }

        AVPacket apkt;
        apkt.data = NULL;
        apkt.size = 0;
        av_init_packet(&apkt);

        ret = avcodec_encode_audio2(audio_st->codec, &apkt, audio_frame, &got_audio);
        if (ret < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error encoding audio frame");
            return 1;
        }

        if (got_audio) {
            /* We have an encoder output to write */
            apkt.pts = av_rescale_q_rnd(apkt.pts, audio_st->codec->time_base, audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.dts = av_rescale_q_rnd(apkt.dts, audio_st->codec->time_base, audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.duration = av_rescale_q(apkt.duration, audio_st->codec->time_base, audio_st->time_base);
            apkt.stream_index = audio_st->index;
            if (av_interleaved_write_frame(formatContext, &apkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return 1;
            }
            av_free_packet(&apkt);
        }

    }

    /* Write file trailer */
    av_write_trailer(formatContext);

    /* Free resources */
    avio_close(formatContext->pb);
    avcodec_close(video_st->codec);
    avcodec_close(audio_st->codec);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    av_freep(&video_frame->data[0]);
    av_frame_free(&video_frame);
    av_frame_free(&audio_frame);

    return 0;
}

#endif

