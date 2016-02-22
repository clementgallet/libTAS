#include "dumpvideo.h"

FILE *f;
const char filename[] = "test.h264";
AVFrame *frame;
AVPacket pkt;
AVCodecContext *c= NULL;

void openVideoDump(void* window) {

    /* Get information about the current screen */
    int width, height;
    SDL_GL_GetDrawableSize_real(window, &width, &height);

    int codec_id = AV_CODEC_ID_H264;
    AVCodec *codec;
    int ret;

    printf("Encode video file %s\n", filename);

    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 400000;

    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;

    /* frames per second */
    c->time_base = (AVRational){1,60};
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_RGBA;

    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }

}

void encodeOneFrame(unsigned long fcounter) {

    int got_output;

    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    fflush(stdout);

    /* Get current screen pixels into the frame struct */
    glReadPixels_real(0, 0, frame->width, frame->height, 0x1908 /* GL_RGBA */, 0x1401 /* GL_UNSIGNED_BYTE */, frame->data);

    frame->pts = fcounter;

    /* encode the image */
    int ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }
    if (got_output) {
        printf("Write frame %ld (size=%5d)\n", fcounter, pkt.size);
        fwrite(pkt.data, 1, pkt.size, f);
        av_free_packet(&pkt);
    }
}


void closeVideoDump() {
    for (int got_output = 1; got_output;) {
        fflush(stdout);

        /* encode the image */
        int ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", -1, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* add sequence end code to have a real mpeg file */
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");
}

