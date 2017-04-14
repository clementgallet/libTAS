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

#include "AVEncoder.h"
#ifdef LIBTAS_ENABLE_AVDUMPING

#include "hook.h"
#include "logging.h"
#include "videocapture.h"
#include "audio/AudioContext.h"
#include "../shared/tasflags.h"
#include "ThreadState.h"

AVEncoder::AVEncoder(void* window, bool video_opengl, char* dumpfile, unsigned long sf) {
    error = 0;

    if (tasflags.framerate <= 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Not supporting non deterministic timer");
        error = 1;
        return;
    }

    start_frame = sf;
    accum_samples = 0;

    int width, height;
    AVPixelFormat pixfmt = initVideoCapture(window, video_opengl, &width, &height);
    if (pixfmt == AV_PIX_FMT_NONE) {
        debuglog(LCF_DUMP | LCF_ERROR, "Unable to initialize video capture");
        error = 1;
        return;
    }

    /* Initialize AVCodec and AVFormat libraries */
    av_register_all();

    /* Initialize AVOutputFormat */
    outputFormat = av_guess_format(NULL, dumpfile, NULL);
    if (!outputFormat) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not find suitable output format for file ", dumpfile);
        error = 1;
        return;
    }

    /* Initialize AVFormatContext */

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize AVFormatContext");
        error = 1;
        return;
    }
    formatContext->oformat = outputFormat;

    /*** Create video stream ***/

    /* Initialize video AVCodec */

    AVCodec *video_codec = NULL;
    // AVCodecID codec_id = AV_CODEC_ID_MPEG4;
    //AVCodecID codec_id = AV_CODEC_ID_H264;
    AVCodecID codec_id = AV_CODEC_ID_FFV1;
    video_codec = avcodec_find_encoder(codec_id);
    if (!video_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Video codec not found");
        error = 1;
        return;
    }
    outputFormat->video_codec = codec_id;

    /* Initialize video stream */

    video_st = avformat_new_stream(formatContext, video_codec);
    if (!video_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        error = 1;
        return;
    }

    /* Fill video stream parameters */
    video_st->id = formatContext->nb_streams - 1;
    video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_st->codec->codec_id = codec_id;

    video_st->codec->bit_rate = 400000;
    video_st->codec->width = width;
    video_st->codec->height = height;
    video_st->time_base = (AVRational){1,static_cast<int>(tasflags.framerate)};
    video_st->codec->time_base = (AVRational){1,static_cast<int>(tasflags.framerate)};
    video_st->codec->gop_size = 10; /* emit one intra frame every ten frames */
    video_st->codec->max_b_frames = 1;
    if (codec_id == AV_CODEC_ID_H264)
        video_st->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_FFV1)
        video_st->codec->pix_fmt = AV_PIX_FMT_YUV444P10LE;

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Use a preset for h264 */
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_st->codec->priv_data, "preset", "slow", 0);

    /* Open the codec */
    if (avcodec_open2(video_st->codec, video_codec, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video codec");
        error = 1;
        return;
    }

    /*** Create audio stream ***/

    /* Initialize audio AVCodec */

    AVCodec *audio_codec = NULL;
    //AVCodecID audio_codec_id = AV_CODEC_ID_PCM_S16LE;
    //AVCodecID audio_codec_id = AV_CODEC_ID_VORBIS;
    //AVCodecID audio_codec_id = AV_CODEC_ID_OPUS;
    AVCodecID audio_codec_id = AV_CODEC_ID_FLAC;
    audio_codec = avcodec_find_encoder(audio_codec_id);
    if (!audio_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Audio codec not found");
        error = 1;
        return;
    }

    /* Initialize audio stream */

    audio_st = avformat_new_stream(formatContext, audio_codec);
    if (!audio_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        error = 1;
        return;
    }

    /* Fill audio stream parameters */

    audio_st->id = formatContext->nb_streams - 1;
    audio_st->codec->codec_type = AVMEDIA_TYPE_AUDIO;

    AVSampleFormat in_fmt = (audiocontext.outBitDepth == 8)?AV_SAMPLE_FMT_U8:AV_SAMPLE_FMT_S16;

    switch (audio_codec_id) {
        case AV_CODEC_ID_VORBIS:
            audio_st->codec->sample_fmt = AV_SAMPLE_FMT_FLTP;
            break;
        case AV_CODEC_ID_OPUS:
        case AV_CODEC_ID_PCM_S16LE:
        case AV_CODEC_ID_FLAC:
            audio_st->codec->sample_fmt = AV_SAMPLE_FMT_S16;
            break;
        default:
            debuglog(LCF_DUMP | LCF_ERROR, "Unknown audio format");
            error = 1;
            return;
    }
    audio_st->codec->bit_rate = 64000;
    audio_st->codec->sample_rate = audiocontext.outFrequency;
    audio_st->codec->channels = audiocontext.outNbChannels;
    audio_st->codec->channel_layout = av_get_default_channel_layout( audio_st->codec->channels );

    /* Some formats want stream headers to be separate. */

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audio_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Open the codec */
    if (avcodec_open2(audio_st->codec, audio_codec, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open audio codec");
        error = 1;
        return;
    }

    /* Initialize video AVFrame */

    video_frame = av_frame_alloc();
    if (!video_frame) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate AVFrame");
        error = 1;
        return;
    }
    video_frame->format = video_st->codec->pix_fmt;
    video_frame->width  = video_st->codec->width;
    video_frame->height = video_st->codec->height;

    /* Allocate the image buffer inside the AVFrame */

    int ret = av_image_alloc(video_frame->data, video_frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate raw picture buffer");
        error = 1;
        return;
    }

    /* Initialize swscale context for pixel format conversion */

    toYUVctx = sws_getContext(video_frame->width, video_frame->height,
                              pixfmt,
                              video_frame->width, video_frame->height,
                              video_st->codec->pix_fmt,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    if (toYUVctx == NULL) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate swscale context");
        error = 1;
        return;
    }

    /* Initialize audio AVFrame */

    audio_frame = av_frame_alloc();

    /* Initialize swscale context for audio format conversion */

    if (in_fmt != audio_st->codec->sample_fmt) {
        audio_fmt_ctx = swr_alloc();
        av_opt_set_int(audio_fmt_ctx, "in_channel_layout",  audio_st->codec->channel_layout, 0);
        av_opt_set_int(audio_fmt_ctx, "out_channel_layout", audio_st->codec->channel_layout,  0);
        av_opt_set_int(audio_fmt_ctx, "in_sample_rate",     audio_st->codec->sample_rate, 0);
        av_opt_set_int(audio_fmt_ctx, "out_sample_rate",    audio_st->codec->sample_rate, 0);
        av_opt_set_sample_fmt(audio_fmt_ctx, "in_sample_fmt",  in_fmt, 0);
        av_opt_set_sample_fmt(audio_fmt_ctx, "out_sample_fmt", audio_st->codec->sample_fmt,  0);
        if (swr_init(audio_fmt_ctx) < 0)
            debuglog(LCF_DUMP | LCF_ERROR, "Error initializing swr context");
    }

    /* Print informations on input and output streams */
    threadState.setOwnCode(true); // We protect the following code because it performs IO that we hook
    av_dump_format(formatContext, 0, dumpfile, 1);

    /* Set up output file */
    if (avio_open(&formatContext->pb, dumpfile, AVIO_FLAG_WRITE) < 0) {
        threadState.setOwnCode(false);
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video file");
        error = 1;
        return;
    }

    /* Write header */
    if (avformat_write_header(formatContext, NULL) < 0) {
        threadState.setOwnCode(false);
        debuglog(LCF_DUMP | LCF_ERROR, "Could not write header");
        error = 1;
        return;
    }

    threadState.setOwnCode(false);
}

/*
 * Encode one frame and send it to the muxer
 * Returns 0 if no error was encountered
 */
int AVEncoder::encodeOneFrame(unsigned long fcounter) {

    /* Check if there was an error during the init */
    if (error)
        return 0;

    /*** Video ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode a video frame");

    const uint8_t* orig_plane[4] = {0};
    int orig_stride[4] = {0};

    /* Access to the screen pixels */
    captureVideoFrame(orig_plane, orig_stride);

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
        vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_st->codec->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_st->codec->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.duration = av_rescale_q(vpkt.duration, video_st->codec->time_base, video_st->time_base);
        vpkt.stream_index = video_st->index;
        if (av_interleaved_write_frame(formatContext, &vpkt) < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
            return 1;
        }
        av_free_packet(&vpkt);
    }

    /*** Audio ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode audio frames");

    /* Append input buffer to our delayed buffer */
    delayed_buffer.insert(delayed_buffer.end(), &audiocontext.outSamples[0], &audiocontext.outSamples[audiocontext.outBytes]);

    if (audio_st->codec->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        audio_frame->nb_samples = audiocontext.outNbSamples;
    else {
        audio_frame->nb_samples = audio_st->codec->frame_size;
    }

    /* Encode loop for every audio frame, until we don't have enough samples */
    while (static_cast<int>(delayed_buffer.size()) >= audio_frame->nb_samples*audiocontext.outAlignSize) {

        /* Initialize AVPacket */
        AVPacket apkt;
        apkt.data = NULL;
        apkt.size = 0;
        av_init_packet(&apkt);

        audio_frame->pts = av_rescale_q(accum_samples, AVRational{1, audio_st->codec->sample_rate}, audio_st->codec->time_base);
        accum_samples += audio_frame->nb_samples;

        /* If necessary, convert the audio stream to the new sample format */
        if (audio_fmt_ctx) {
            int line_size;
            int buf_size = av_samples_get_buffer_size(&line_size, audio_st->codec->channels, audio_frame->nb_samples, audio_st->codec->sample_fmt, 1);
            temp_audio.resize(buf_size);

            /* Build the lines array for planar sample format */
            int lines_nb = av_sample_fmt_is_planar(audio_st->codec->sample_fmt)?audio_st->codec->channels:1;
            std::vector<uint8_t*> lines;
            lines.resize(lines_nb);
            for (int c=0; c<lines_nb; c++)
                lines[c] = temp_audio.data() + c*line_size;
            uint8_t* in_pt = delayed_buffer.data();

            swr_convert(audio_fmt_ctx, lines.data(), audio_frame->nb_samples, const_cast<const uint8_t**>(&in_pt), audio_frame->nb_samples);
            avcodec_fill_audio_frame(audio_frame, audio_st->codec->channels, audio_st->codec->sample_fmt,
                                                     lines[0], buf_size, 1);
        }
        else {
            avcodec_fill_audio_frame(audio_frame, audio_st->codec->channels, audio_st->codec->sample_fmt,
                                                 delayed_buffer.data(), audio_frame->nb_samples*audiocontext.outAlignSize, 1);
        }

        ret = avcodec_encode_audio2(audio_st->codec, &apkt, audio_frame, &got_output);
        if (ret < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error encoding audio frame");
            return 1;
        }

        if (got_output) {
            /* We have an encoder output to write */
            apkt.pts = av_rescale_q_rnd(apkt.pts, audio_st->codec->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.dts = av_rescale_q_rnd(apkt.dts, audio_st->codec->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.duration = av_rescale_q(apkt.duration, audio_st->codec->time_base, audio_st->time_base);
            apkt.stream_index = audio_st->index;
            if (av_interleaved_write_frame(formatContext, &apkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return 1;
            }
            av_free_packet(&apkt);
        }

        delayed_buffer.erase(delayed_buffer.begin(), delayed_buffer.begin()+audio_frame->nb_samples*audiocontext.outAlignSize);
    }

    return 0;
}

AVEncoder::~AVEncoder() {
    /* Check if there was an error during the init */
    if (error)
        return;

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
            return;
        }

        if (got_video) {
            vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_st->codec->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_st->codec->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.duration = av_rescale_q(vpkt.duration, video_st->codec->time_base, video_st->time_base);
            vpkt.stream_index = video_st->index;
            if (av_interleaved_write_frame(formatContext, &vpkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return;
            }
            av_free_packet(&vpkt);
        }

        AVPacket apkt;
        apkt.data = NULL;
        apkt.size = 0;
        av_init_packet(&apkt);

        ret = avcodec_encode_audio2(audio_st->codec, &apkt, NULL, &got_audio);
        if (ret < 0) {
            debuglog(LCF_DUMP | LCF_ERROR, "Error encoding audio frame");
            return;
        }

        if (got_audio) {
            /* We have an encoder output to write */
            apkt.pts = av_rescale_q_rnd(apkt.pts, audio_st->codec->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.dts = av_rescale_q_rnd(apkt.dts, audio_st->codec->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.duration = av_rescale_q(apkt.duration, audio_st->codec->time_base, audio_st->time_base);
            apkt.stream_index = audio_st->index;
            if (av_interleaved_write_frame(formatContext, &apkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return;
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
}

#endif
