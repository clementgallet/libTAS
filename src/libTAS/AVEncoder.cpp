/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "ScreenCapture.h"
#include "audio/AudioContext.h"
#include "global.h" // shared_config
#include "GlobalState.h"

#define ASSERT_RETURN_VOID(expr, msg) do { \
        if (!(expr)) { \
            error_msg = msg; \
            debuglog(LCF_DUMP | LCF_ERROR, msg); \
            error = -1; \
            return; \
        } \
    } while (false)

#define ASSERT_RETURN(ret, msg) do { \
        if (ret < 0) { \
            error_msg = msg; \
            debuglog(LCF_DUMP | LCF_ERROR, msg); \
            return ret; \
        } \
    } while (false)

namespace libtas {

static std::string error_msg;

AVEncoder::AVEncoder(SDL_Window* window, unsigned long sf) {
    error = 0;

    ASSERT_RETURN_VOID(shared_config.framerate_num > 0, "Not supporting non deterministic timer");

    start_frame = sf;
    accum_samples = 0;

    int width, height, ret;
    ScreenCapture::getDimensions(width, height);

    AVPixelFormat pixfmt = ScreenCapture::getPixelFormat();
    ASSERT_RETURN_VOID(pixfmt != AV_PIX_FMT_NONE, "Unable to get pixel format");

    /* Initialize AVCodec and AVFormat libraries */
    av_register_all();

    /* Initialize AVOutputFormat */
    outputFormat = av_guess_format(NULL, dumpfile, NULL);
    ASSERT_RETURN_VOID(outputFormat, "Could not find suitable output format");

    /* Initialize AVFormatContext */

    formatContext = avformat_alloc_context();
    ASSERT_RETURN_VOID(formatContext, "Could not initialize AVFormatContext");
    formatContext->oformat = outputFormat;

    /*** Create video stream ***/

    /* Initialize video AVCodec */

    AVCodecID codec_id = shared_config.video_codec;
    AVCodec *video_codec = avcodec_find_encoder(codec_id);
    ASSERT_RETURN_VOID(video_codec, "Video codec not found");
    outputFormat->video_codec = codec_id;

    /* Initialize video stream */

    video_st = avformat_new_stream(formatContext, video_codec);
    ASSERT_RETURN_VOID(video_st, "Could not initialize video AVStream");
    video_st->id = formatContext->nb_streams - 1;

    /* Initialize video codec parameters */
    video_codec_context = avcodec_alloc_context3(video_codec);
    ASSERT_RETURN_VOID(video_codec_context, "Could not alloc an encoding context");

    /* Fill video stream parameters */
    // video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_codec_context->codec_id = codec_id;
    video_codec_context->bit_rate = shared_config.video_bitrate;
    video_codec_context->width = width;
    video_codec_context->height = height;
    video_codec_context->time_base = (AVRational){static_cast<int>(shared_config.framerate_den),static_cast<int>(shared_config.framerate_num)};
    video_codec_context->gop_size = 10; /* emit one intra frame every ten frames */
    // video_codec_context->max_b_frames = 1;

    /* Checking if we can use the native pixel format.
     * If not, taking the default pixel format of the codec
     */
    video_codec_context->pix_fmt = pixfmt;
    if (video_codec_context->codec->pix_fmts) {
        int i;
        for (i = 0; video_codec_context->codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++) {
            if (video_codec_context->pix_fmt == video_codec_context->codec->pix_fmts[i]) {
                /* Our native format is accepted by the codec */
                break;
            }
        }

        if (video_codec_context->codec->pix_fmts[i] == AV_PIX_FMT_NONE) {
            /* Our native pixel format is not supported,
             * taking the default one.
             */

            video_codec_context->pix_fmt = avcodec_default_get_format(video_codec_context, video_codec_context->codec->pix_fmts);
            debuglog(LCF_DUMP, "Our native pixel format is not supported. Choosing ", av_get_pix_fmt_name(video_codec_context->pix_fmt));
        }
    }

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* Use a preset for h264 */
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_codec_context->priv_data, "preset", "slow", 0);

    /* Open the codec */

    ret = avcodec_open2(video_codec_context, NULL, NULL);
    ASSERT_RETURN_VOID(ret >= 0, "Could not open video codec");

    /* Copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(video_st->codecpar, video_codec_context);
    ASSERT_RETURN_VOID(ret >= 0, "Could not copy the stream parameters");

    /*** Create audio stream ***/

    /* Initialize audio AVCodec */

    AVCodecID audio_codec_id = shared_config.audio_codec;
    AVCodec *audio_codec = avcodec_find_encoder(audio_codec_id);
    ASSERT_RETURN_VOID(audio_codec, "Audio codec not found");

    /* Initialize audio stream */

    audio_st = avformat_new_stream(formatContext, audio_codec);
    ASSERT_RETURN_VOID(audio_st, "Could not initialize video AVStream");
    audio_st->id = formatContext->nb_streams - 1;

    /* Initialize audio codec parameters */
    audio_codec_context = avcodec_alloc_context3(audio_codec);
    ASSERT_RETURN_VOID(audio_codec_context, "Could not alloc an encoding context");

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
    audio_codec_context->time_base = (AVRational){ 1, audio_codec_context->sample_rate };
    // audio_st->time_base = (AVRational){ 1, audio_codec_context->sample_rate };

    /* Some formats want stream headers to be separate. */

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audio_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* Open the codec */
    ret = avcodec_open2(audio_codec_context, NULL, NULL);
    ASSERT_RETURN_VOID(ret >= 0, "Could not open audio codec");

    /* Copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(audio_st->codecpar, audio_codec_context);
    ASSERT_RETURN_VOID(ret >= 0, "Could not copy the stream parameters");

    /* Initialize video AVFrame */

    video_frame = av_frame_alloc();
    ASSERT_RETURN_VOID(video_frame, "Could not allocate AVFrame");

    video_frame->format = video_codec_context->pix_fmt;
    video_frame->width  = video_codec_context->width;
    video_frame->height = video_codec_context->height;

    /* Allocate the image buffer inside the AVFrame */
    ret = av_frame_get_buffer(video_frame, 32);
    ASSERT_RETURN_VOID(ret >= 0, "Could not allocate raw picture buffer");

    /* Initialize swscale context for pixel format conversion */

    toYUVctx = sws_getContext(video_frame->width, video_frame->height,
                              pixfmt,
                              video_frame->width, video_frame->height,
                              video_codec_context->pix_fmt,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    ASSERT_RETURN_VOID(toYUVctx, "Could not allocate swscale context");

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

        ret = swr_init(audio_fmt_ctx);
        ASSERT_RETURN_VOID(ret >= 0, "Error initializing swr context");
    }

    /* Print informations on input and output streams */
    GlobalState::setOwnCode(true); // We protect the following code because it performs IO that we hook
    av_dump_format(formatContext, 0, dumpfile, 1);

    /* Set up output file */
    if (avio_open(&formatContext->pb, dumpfile, AVIO_FLAG_WRITE) < 0) {
        GlobalState::setOwnCode(false);
        error_msg = "Could not open video file";
        debuglog(LCF_DUMP | LCF_ERROR, error_msg);
        error = -1;
        return;
    }

    /* Write header */
    if (avformat_write_header(formatContext, NULL) < 0) {
        GlobalState::setOwnCode(false);
        error_msg = "Could not write header";
        debuglog(LCF_DUMP | LCF_ERROR, error_msg);
        error = -1;
        return;
    }

    GlobalState::setOwnCode(false);
}

std::string AVEncoder::getErrorMsg() {
    return error_msg;
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
     * or if we never drew. If not, we will capture the last drawn frame.
     */
    if (draw || (orig_stride[0] == 0)) {
        ScreenCapture::getPixels(orig_plane, orig_stride);

        /* Change pixel format to YUV420p and copy it into the AVframe */
        ret = sws_scale(toYUVctx, orig_plane, orig_stride, 0,
                    video_frame->height, video_frame->data, video_frame->linesize);
        if (ret != video_frame->height) {
            error_msg = "We could not rescale the video frame";
            debuglog(LCF_DUMP | LCF_ERROR, error_msg);
            return -1;
        }
    }

    video_frame->pts = fcounter - start_frame;

    /* Encode the image */
    ret = avcodec_send_frame(video_codec_context, video_frame);
    ASSERT_RETURN(ret, "Error encoding video frame");

    /* Receive decoding frames */
    ret = receive_packet(video_codec_context, video_st);
    if (ret < 0) return ret;

    /*** Audio ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode audio frames");

    /* Append input buffer to our delayed buffer */
    delayed_buffer.insert(delayed_buffer.end(), &audiocontext.outSamples[0], &audiocontext.outSamples[audiocontext.outBytes]);

    if (audio_codec_context->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        audio_frame->nb_samples = audiocontext.outNbSamples;
    else {
        audio_frame->nb_samples = audio_codec_context->frame_size;
    }
    /* FIXME: If sample rate change by resampling, the following code is wrong */
    /* Encode loop for every audio frame, until we don't have enough samples */
    while (static_cast<int>(delayed_buffer.size()) >= audio_frame->nb_samples*audiocontext.outAlignSize) {
        ret = send_audio_frame();
        if (ret < 0) return ret;

        /* Receive decoding frames */
        ret = receive_packet(audio_codec_context, audio_st);
        if (ret < 0) return ret;
    }

    return 0;
}

int AVEncoder::send_audio_frame()
{
    int ret;
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

    return 0;
}

int AVEncoder::receive_packet(AVCodecContext *codec_context, AVStream* st)
{
    AVPacket pkt = { 0 };
    int ret = avcodec_receive_packet(codec_context, &pkt);

    while (ret == 0) {
        /* We have an encoder output to write */
        pkt.pts = av_rescale_q_rnd(pkt.pts, codec_context->time_base, st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, codec_context->time_base, st->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, codec_context->time_base, st->time_base);
        pkt.stream_index = st->index;
        ret = av_interleaved_write_frame(formatContext, &pkt);
        ASSERT_RETURN(ret, "Error writing frame");

        ret = avcodec_receive_packet(codec_context, &pkt);
    }

    // if (ret != AVERROR(EAGAIN)) {
    //     ASSERT_RETURN(ret, "Error getting paquet");
    // }

    return 0;
}

AVEncoder::~AVEncoder() {
    /* Check if there was an error during the init */
    if (error)
        return;

    /* Send the rest of audio samples */
    audio_frame->nb_samples = delayed_buffer.size() / audiocontext.outAlignSize;
    send_audio_frame();

    /* Tells the encoder to flush the streams */
    avcodec_send_frame(video_codec_context, nullptr);
    avcodec_send_frame(audio_codec_context, nullptr);

    debuglog(LCF_DUMP, "Start getting flushed frames");

    /* Encode the remaining frames */
    receive_packet(video_codec_context, video_st);
    receive_packet(audio_codec_context, audio_st);

    /* Flush the interleaving queue, needed? */
    av_interleaved_write_frame(formatContext, NULL);

    /* Write file trailer */
    av_write_trailer(formatContext);

    /* Free resources */
    avcodec_free_context(&video_codec_context);
    avcodec_free_context(&audio_codec_context);
    avio_close(formatContext->pb);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    if (audio_fmt_ctx)
        swr_free(&audio_fmt_ctx);
    // av_freep(&video_frame->data[0]);
    av_frame_free(&video_frame);
    av_frame_free(&audio_frame);
}

char AVEncoder::dumpfile[4096] = {0};

std::unique_ptr<AVEncoder> avencoder;

}

#endif
