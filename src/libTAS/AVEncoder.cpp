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
#include "screenpixels.h"
#include "audio/AudioContext.h"
#include "../shared/SharedConfig.h"
#include "GlobalState.h"

#define ASSERT_RETURN(ret, msg) do { \
        if (ret < 0) { \
            debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame"); \
            return ret; \
        } \
    } while (false)

AVEncoder::AVEncoder(SDL_Window* window, bool video_opengl, const char* dumpfile, unsigned long sf) {
    error = 0;

    if (shared_config.framerate <= 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Not supporting non deterministic timer");
        error = -1;
        return;
    }

    start_frame = sf;
    accum_samples = 0;

    int width, height;
    int ret = initScreenPixels(window, video_opengl, &width, &height);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Unable to initialize video capture");
        error = -1;
        return;
    }

    AVPixelFormat pixfmt = getPixelFormat(window);
    if (pixfmt == AV_PIX_FMT_NONE) {
        debuglog(LCF_DUMP | LCF_ERROR, "Unable to get pixel format");
        error = -1;
        return;
    }

    /* Initialize AVCodec and AVFormat libraries */
    av_register_all();

    /* Initialize AVOutputFormat */
    outputFormat = av_guess_format(NULL, dumpfile, NULL);
    if (!outputFormat) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not find suitable output format for file ", dumpfile);
        error = -1;
        return;
    }

    /* Initialize AVFormatContext */

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize AVFormatContext");
        error = -1;
        return;
    }
    formatContext->oformat = outputFormat;

    /*** Create video stream ***/

    /* Initialize video AVCodec */

    AVCodecID codec_id = shared_config.video_codec;
    AVCodec *video_codec = avcodec_find_encoder(codec_id);
    if (!video_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Video codec not found");
        error = -1;
        return;
    }
    outputFormat->video_codec = codec_id;

    /* Initialize video stream */

    video_st = avformat_new_stream(formatContext, video_codec);
    if (!video_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        error = -1;
        return;
    }
    video_st->id = formatContext->nb_streams - 1;

    /* Initialize video codec parameters */
    video_codec_context = avcodec_alloc_context3(video_codec);
    if (!video_codec_context) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not alloc an encoding context");
        return;
    }

    /* Fill video stream parameters */
    // video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_codec_context->codec_id = codec_id;
    video_codec_context->bit_rate = shared_config.video_bitrate;
    video_codec_context->width = width;
    video_codec_context->height = height;
    video_st->time_base = (AVRational){1,static_cast<int>(shared_config.framerate)};
    video_codec_context->time_base = video_st->time_base;
    video_codec_context->gop_size = 10; /* emit one intra frame every ten frames */
    // video_codec_context->max_b_frames = 1;
    if (codec_id == AV_CODEC_ID_H264)
        video_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_FFV1)
        video_codec_context->pix_fmt = AV_PIX_FMT_YUV444P10LE;
    if (codec_id == AV_CODEC_ID_RAWVIDEO)
        video_codec_context->pix_fmt = pixfmt;

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Use a preset for h264 */
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_codec_context->priv_data, "preset", "slow", 0);

    /* Open the codec */
    if (avcodec_open2(video_codec_context, NULL, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video codec");
        error = -1;
        return;
    }

    /* Copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(video_st->codecpar, video_codec_context);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not copy the stream parameters");
        error = -1;
        return;
    }

    /*** Create audio stream ***/

    /* Initialize audio AVCodec */

    AVCodecID audio_codec_id = shared_config.audio_codec;
    AVCodec *audio_codec = avcodec_find_encoder(audio_codec_id);
    if (!audio_codec) {
        debuglog(LCF_DUMP | LCF_ERROR, "Audio codec not found");
        error = -1;
        return;
    }

    /* Initialize audio stream */

    audio_st = avformat_new_stream(formatContext, audio_codec);
    if (!audio_st) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not initialize video AVStream");
        error = -1;
        return;
    }
    audio_st->id = formatContext->nb_streams - 1;

    /* Initialize audio codec parameters */
    audio_codec_context = avcodec_alloc_context3(audio_codec);
    if (!audio_codec_context) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not alloc an encoding context");
        return;
    }

    /* Fill audio stream parameters */

    AVSampleFormat in_fmt = (audiocontext.outBitDepth == 8)?AV_SAMPLE_FMT_U8:AV_SAMPLE_FMT_S16;
    audio_codec_context->sample_fmt = in_fmt;

    /* Checking if we can use the native sample format.
     * If not, taking a sample format supported by the codec
     */
    if (audio_codec_context->codec->sample_fmts) {
        int i;
        for (i = 0; audio_codec_context->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
            if (audio_codec_context->sample_fmt == audio_codec_context->codec->sample_fmts[i])
                /* Our native format is accepted by the codec */
                break;
            if (audio_codec_context->channels == 1 &&
            av_get_planar_sample_fmt(audio_codec_context->sample_fmt) == av_get_planar_sample_fmt(audio_codec_context->codec->sample_fmts[i])) {
                /* If don't care about plannar status if we only have one channel. */
                audio_codec_context->sample_fmt = audio_codec_context->codec->sample_fmts[i];
                break;
            }
        }

        if (audio_codec_context->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) {
            /* Our native sample format is not supported,
             * taking the first supported one.
             */
            audio_codec_context->sample_fmt = audio_codec_context->codec->sample_fmts[0];
            debuglog(LCF_DUMP, "Our native sample format is not supported. Choosing ", av_get_sample_fmt_name(audio_codec_context->sample_fmt));
        }
    }

    /* Checking if we can use the native sample rate.
     * If not, taking a sample rate supported by the codec
     */
    audio_codec_context->sample_rate = audiocontext.outFrequency;

    if (audio_codec_context->codec->supported_samplerates) {
        int i;
        for (i = 0; audio_codec_context->codec->supported_samplerates[i] != 0; i++) {
            if (audio_codec_context->sample_rate == audio_codec_context->codec->supported_samplerates[i])
                /* Our native sample rate is accepted by the codec */
                break;
        }

        if (audio_codec_context->codec->supported_samplerates[i] == 0) {
            /* Our native sample rate is not supported,
             * taking the first supported one.
             */
            audio_codec_context->sample_rate = audio_codec_context->codec->supported_samplerates[0];
            debuglog(LCF_DUMP, "Our native sample rate is not supported. Choosing ", audio_codec_context->sample_rate);
        }
    }

    audio_codec_context->bit_rate = shared_config.audio_bitrate;
    audio_codec_context->channels = audiocontext.outNbChannels;
    audio_codec_context->channel_layout = av_get_default_channel_layout( audio_codec_context->channels );
    audio_st->time_base = (AVRational){ 1, audio_codec_context->sample_rate };

    /* Some formats want stream headers to be separate. */

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audio_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* Open the codec */
    if (avcodec_open2(audio_codec_context, NULL, NULL) < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open audio codec");
        error = -1;
        return;
    }

    /* Copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(audio_st->codecpar, audio_codec_context);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not copy the stream parameters");
        error = -1;
        return;
    }

    /* Initialize video AVFrame */

    video_frame = av_frame_alloc();
    if (!video_frame) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate AVFrame");
        error = -1;
        return;
    }
    video_frame->format = video_codec_context->pix_fmt;
    video_frame->width  = video_codec_context->width;
    video_frame->height = video_codec_context->height;

    /* Allocate the image buffer inside the AVFrame */

    ret = av_image_alloc(video_frame->data, video_frame->linesize, video_codec_context->width, video_codec_context->height, video_codec_context->pix_fmt, 32);
    if (ret < 0) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate raw picture buffer");
        error = -1;
        return;
    }

    /* Initialize swscale context for pixel format conversion */

    toYUVctx = sws_getContext(video_frame->width, video_frame->height,
                              pixfmt,
                              video_frame->width, video_frame->height,
                              video_codec_context->pix_fmt,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    if (toYUVctx == NULL) {
        debuglog(LCF_DUMP | LCF_ERROR, "Could not allocate swscale context");
        error = -1;
        return;
    }

    /* Initialize audio AVFrame */

    audio_frame = av_frame_alloc();

    /* Initialize swscale context for audio format conversion */
    if ((in_fmt != audio_codec_context->sample_fmt) || (audio_codec_context->sample_rate != audiocontext.outFrequency)) {
        audio_fmt_ctx = swr_alloc();
        av_opt_set_int(audio_fmt_ctx, "in_channel_layout",  audio_codec_context->channel_layout, 0);
        av_opt_set_int(audio_fmt_ctx, "out_channel_layout", audio_codec_context->channel_layout,  0);
        av_opt_set_int(audio_fmt_ctx, "in_sample_rate",     audiocontext.outFrequency, 0);
        av_opt_set_int(audio_fmt_ctx, "out_sample_rate",    audio_codec_context->sample_rate, 0);
        av_opt_set_sample_fmt(audio_fmt_ctx, "in_sample_fmt",  in_fmt, 0);
        av_opt_set_sample_fmt(audio_fmt_ctx, "out_sample_fmt", audio_codec_context->sample_fmt,  0);
        if (swr_init(audio_fmt_ctx) < 0)
            debuglog(LCF_DUMP | LCF_ERROR, "Error initializing swr context");
    }

    /* Print informations on input and output streams */
    GlobalState::setOwnCode(true); // We protect the following code because it performs IO that we hook
    av_dump_format(formatContext, 0, dumpfile, 1);

    /* Set up output file */
    if (avio_open(&formatContext->pb, dumpfile, AVIO_FLAG_WRITE) < 0) {
        GlobalState::setOwnCode(false);
        debuglog(LCF_DUMP | LCF_ERROR, "Could not open video file");
        error = -1;
        return;
    }

    /* Write header */
    if (avformat_write_header(formatContext, NULL) < 0) {
        GlobalState::setOwnCode(false);
        debuglog(LCF_DUMP | LCF_ERROR, "Could not write header");
        error = -1;
        return;
    }

    GlobalState::setOwnCode(false);
}

/*
 * Encode one frame and send it to the muxer
 * Returns 0 if no error was encountered, or a negative value if an error
 * was encountered.
 */
int AVEncoder::encodeOneFrame(unsigned long fcounter, bool draw) {
    int ret;

    /* Check if there was an error during the init */
    if (error != 0)
        return error;

    /*** Video ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode a video frame");

    static const uint8_t* orig_plane[4] = {0};
    static int orig_stride[4] = {0};

    /* Access to the screen pixels if the current frame is a draw frame
     * if not, we will capture the last drawn frame.
     */
    if (draw) {
        getScreenPixels(orig_plane, orig_stride);

        /* Change pixel format to YUV420p and copy it into the AVframe */
        ret = sws_scale(toYUVctx, orig_plane, orig_stride, 0,
                    video_frame->height, video_frame->data, video_frame->linesize);
        if (ret != video_frame->height) {
            debuglog(LCF_DUMP | LCF_ERROR, "We could only convert ",ret," rows");
            return -1;
        }

        video_frame->pts = fcounter - start_frame;

        /* Encode the image */
        ret = avcodec_send_frame(video_codec_context, video_frame);
        ASSERT_RETURN(ret, "Error encoding video frame");
    }

    /* Receive decoding frames */
    AVPacket vpkt = { 0 };

    ret = avcodec_receive_packet(video_codec_context, &vpkt);
    while (ret == 0) {
        /* Rescale output packet timestamp values from codec to stream timebase */
        vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_codec_context->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_codec_context->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        vpkt.duration = av_rescale_q(vpkt.duration, video_codec_context->time_base, video_st->time_base);
        vpkt.stream_index = video_st->index;
        ret = av_interleaved_write_frame(formatContext, &vpkt);

        ret = avcodec_receive_packet(video_codec_context, &vpkt);
        // av_free_packet(&vpkt);
    }

    if (ret != AVERROR(EAGAIN)) {
        ASSERT_RETURN(ret, "Error getting video paquet");
    }

    /*** Audio ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode audio frames");

    /* Append input buffer to our delayed buffer */
    delayed_buffer.insert(delayed_buffer.end(), &audiocontext.outSamples[0], &audiocontext.outSamples[audiocontext.outBytes]);

    if (audio_codec_context->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        audio_frame->nb_samples = audiocontext.outNbSamples;
    else {
        audio_frame->nb_samples = audio_codec_context->frame_size;
    }
    /* FIXME: If sample rate change by resampling, the following code is wrong */
    /* Encode loop for every audio frame, until we don't have enough samples */
    while (static_cast<int>(delayed_buffer.size()) >= audio_frame->nb_samples*audiocontext.outAlignSize) {

        audio_frame->pts = av_rescale_q(accum_samples, AVRational{1, audio_codec_context->sample_rate}, audio_codec_context->time_base);

        /* If necessary, convert the audio stream to the new sample format */
        if (audio_fmt_ctx) {
            int line_size;
            int buf_size = av_samples_get_buffer_size(&line_size, audio_codec_context->channels, audio_frame->nb_samples, audio_codec_context->sample_fmt, 1);
            temp_audio.resize(buf_size);

            /* Build the lines array for planar sample format */
            int lines_nb = av_sample_fmt_is_planar(audio_codec_context->sample_fmt)?audio_codec_context->channels:1;
            std::vector<uint8_t*> lines;
            lines.resize(lines_nb);
            for (int c=0; c<lines_nb; c++)
                lines[c] = temp_audio.data() + c*line_size;
            uint8_t* in_pt = delayed_buffer.data();

            ret = swr_convert(audio_fmt_ctx, lines.data(), audio_frame->nb_samples, const_cast<const uint8_t**>(&in_pt), audio_frame->nb_samples);

            ASSERT_RETURN(ret, "Error scaling audio frame");

            ret = avcodec_fill_audio_frame(audio_frame, audio_codec_context->channels, audio_codec_context->sample_fmt,
                                                     lines[0], buf_size, 1);
        }
        else {
            ret = avcodec_fill_audio_frame(audio_frame, audio_codec_context->channels, audio_codec_context->sample_fmt,
                                                 delayed_buffer.data(), audio_frame->nb_samples*audiocontext.outAlignSize, 1);
        }

        ASSERT_RETURN(ret, "Error filling audio frame");

        ret = avcodec_send_frame(audio_codec_context, audio_frame);

        if (ret == AVERROR(EAGAIN)) {
            /* We cannot send more audio data because the internal buffer is full
             * Don't touch our buffer.
             */
        }
        else if (ret < 0) {
            ASSERT_RETURN(ret, "Error encoding audio frame");
        }
        else {
            accum_samples += audio_frame->nb_samples;
            delayed_buffer.erase(delayed_buffer.begin(), delayed_buffer.begin()+audio_frame->nb_samples*audiocontext.outAlignSize);
        }

        /* Receive decoding frames */

        AVPacket apkt = { 0 };
        ret = avcodec_receive_packet(audio_codec_context, &apkt);

        while (ret == 0) {
            /* We have an encoder output to write */
            apkt.pts = av_rescale_q_rnd(apkt.pts, audio_codec_context->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.dts = av_rescale_q_rnd(apkt.dts, audio_codec_context->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.duration = av_rescale_q(apkt.duration, audio_codec_context->time_base, audio_st->time_base);
            apkt.stream_index = audio_st->index;
            ret = av_interleaved_write_frame(formatContext, &apkt);
            ASSERT_RETURN(ret, "Error writing frame");

            ret = avcodec_receive_packet(audio_codec_context, &apkt);
        }

        if (ret != AVERROR(EAGAIN)) {
            ASSERT_RETURN(ret, "Error getting audio paquet");
        }
    }

    return 0;
}

AVEncoder::~AVEncoder() {
    /* Check if there was an error during the init */
    if (error)
        return;

    /* Tells the encoder to flush the streams */
    avcodec_send_frame(video_codec_context, nullptr);
    avcodec_send_frame(audio_codec_context, nullptr);

    debuglog(LCF_DUMP, "Start getting flushed frames");

    /* Encode the remaining frames */
    int vret = 0;
    while (vret == 0) {

        AVPacket vpkt = { 0 };
        vret = avcodec_receive_packet(video_codec_context, &vpkt);

        if (vret == 0) {
            vpkt.pts = av_rescale_q_rnd(vpkt.pts, video_codec_context->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.dts = av_rescale_q_rnd(vpkt.dts, video_codec_context->time_base, video_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            vpkt.duration = av_rescale_q(vpkt.duration, video_codec_context->time_base, video_st->time_base);
            vpkt.stream_index = video_st->index;
            debuglog(LCF_DUMP, "Write a flushed video frame");
            if (av_interleaved_write_frame(formatContext, &vpkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return;
            }
            // av_free_packet(&vpkt);
        }
    }

    int aret = 0;
    while (aret == 0) {

        AVPacket apkt = { 0 };
        // apkt.data = NULL;
        // apkt.size = 0;
        // av_init_packet(&apkt);

        aret = avcodec_receive_packet(audio_codec_context, &apkt);

        if (aret == 0) {
            /* We have an encoder output to write */
            apkt.pts = av_rescale_q_rnd(apkt.pts, audio_codec_context->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.dts = av_rescale_q_rnd(apkt.dts, audio_codec_context->time_base, audio_st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            apkt.duration = av_rescale_q(apkt.duration, audio_codec_context->time_base, audio_st->time_base);
            apkt.stream_index = audio_st->index;
            debuglog(LCF_DUMP, "Write a flushed audio frame");
            if (av_interleaved_write_frame(formatContext, &apkt) < 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Error writing frame");
                return;
            }
            // av_free_packet(&apkt);
        }
    }

    /* Flush the interleaving queue, needed? */
    av_interleaved_write_frame(formatContext, NULL);

    /* Write file trailer */
    av_write_trailer(formatContext);

    /* Free resources */
    avcodec_free_context(&video_codec_context);
    avcodec_free_context(&audio_codec_context);
    avio_close(formatContext->pb);
    // avcodec_close(video_codec);
    // avcodec_close(audio_codec);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    // av_freep(&video_frame->data[0]);
    av_frame_free(&video_frame);
    av_frame_free(&audio_frame);

    finiScreenPixels();
}

#endif
