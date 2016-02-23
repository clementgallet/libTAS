#include "dumpvideo.h"

FILE *f;
const char filename[] = "test.avi";
AVFrame *frame;
AVPacket pkt;
AVCodecContext *c= NULL;
struct SwsContext *toYUVctx = NULL;
AVOutputFormat *outputFormat = NULL;
AVFormatContext *formatContext = NULL;
AVStream* video_st;

int yuvwidth, yuvheight;
int ystride, uvstride;
int ysize, uvsize, yuvsize;

typedef void (*glReadPixelsF)(int, int, int, int, unsigned int, unsigned int, void*);
glReadPixelsF glReadPixels = NULL;

void openVideoDump(void* window) {

    glReadPixels = (glReadPixelsF) SDL_GL_GetProcAddress_real("glReadPixels");
    if (!glReadPixels)
        fprintf(stderr, "Could not load function glReadPixels\n");


    //avcodec_init();
    av_log_set_level( AV_LOG_DEBUG );
    av_register_all();
    avcodec_register_all(); // I should not need this

    /*
    AVCodec * codec2 = av_codec_next(NULL);
    while(codec2 != NULL)
    {
        fprintf(stderr, "%s\n", codec2->long_name);
        codec2 = av_codec_next(codec2);
    }
    */


    /* Get information about the current screen */
    int width, height;
    SDL_GL_GetDrawableSize_real(window, &width, &height);

    int ret;

    printf("Encode video file %s\n", filename);

    /* find the mpeg1 video encoder */
    

    /*
    fprintf(stderr, "Available pixfmt: \n");
    int pi = 0;
    while (codec->pix_fmts[pi] != -1) {
        fprintf(stderr, "pf %d\n", codec->pix_fmts[pi]);
        pi++;
    }
    */


    /*
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    */
    /* Initialize AVFormat */
    
    outputFormat = av_guess_format(NULL, filename, NULL);
    //outputFormat = av_guess_format("mp4", NULL, NULL);
    if (!outputFormat) {
        fprintf(stderr, "Could not find suitable output format\n");
        return;
    }

    // define AVFormatContext
    // lallocate the output media context
    formatContext = avformat_alloc_context();
    //avformat_alloc_output_context2(&formatContext, NULL, NULL, filename);
//  avformat_alloc_output_context(&formatContext, outputFormat, NULL, NULL);
    if (!formatContext) {
        fprintf(stderr, "Memory error\n");
        return;
    }

    //int codec_id = AV_CODEC_ID_H264;
    int codec_id = AV_CODEC_ID_MPEG4;
    outputFormat->video_codec = codec_id;
    formatContext->oformat = outputFormat;

    //int codec_id = AV_CODEC_ID_MPEG4;
    AVCodec *codec = NULL;
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }


    video_st = avformat_new_stream(formatContext, codec);

    fprintf(stderr, "Created new stream\n");

    /*
    avcodec_get_context_defaults3(video_st->codec, codec);*/

    video_st->codec->coder_type = AVMEDIA_TYPE_VIDEO;
    video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_st->codec->codec_id = codec_id;

    /* put sample parameters */
    video_st->codec->bit_rate = 400000;

    /* resolution must be a multiple of two */
    video_st->codec->width = width;
    video_st->codec->height = height;

    /* frames per second */
    //video_st->codec->time_base = (AVRational){1,60};
    video_st->time_base = (AVRational){1,60};
    video_st->codec->time_base.den = 60;
    video_st->codec->time_base.num = 1;
    video_st->codec->gop_size = 12; /* emit one intra frame every ten frames */
    video_st->codec->max_b_frames = 1;
    video_st->codec->pix_fmt = AV_PIX_FMT_YUV420P;

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;


    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_st->codec->priv_data, "preset", "slow", 0);

    if (avcodec_open2(video_st->codec, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    
    
    fprintf(stderr, "Alloc frame\n");
    
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = video_st->codec->pix_fmt;
    frame->width  = video_st->codec->width;
    frame->height = video_st->codec->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }


    /* Initialize swscale for pixel format conversion */
    yuvwidth    = RNDTO2 ( frame->width );
    yuvheight   = RNDTO2 ( frame->height );
    ystride     = RNDTO32 ( yuvwidth );
    uvstride    = RNDTO32 ( yuvwidth / 2 );
    ysize       = ystride * yuvheight;
    uvsize      = uvstride * ( yuvheight / 2 );
    yuvsize     = ysize + ( 2 * uvsize );

    toYUVctx = sws_getContext(frame->width, frame->height,  
                              PIX_FMT_RGBA,
                              yuvwidth, yuvheight, 
                              PIX_FMT_YUV420P,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    if (toYUVctx == NULL) {
        fprintf(stderr, "Could not allocate swscale context\n");
        exit(1);
    }


    av_dump_format(formatContext, 0, filename, 1);
    
    /* Set up output file */
    avio_open(&formatContext->pb, filename, AVIO_FLAG_WRITE);

    /* Write header */
    int rh = avformat_write_header(formatContext, NULL);
    if (rh < 0) {
        fprintf(stderr, "Could not write header!\n");
    }
    
    
    fprintf(stderr, "Successfully inited avcodec and swscale\n");
}

void encodeOneFrame(unsigned long fcounter, void* window) {

    //fprintf(stderr, "Start encoding a frame\n");
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    SDL_Surface* surface = SDL_GetWindowSurface_real(window);
    /* SDL_ConvertPixels? */
    if (0 != SDL_LockSurface_real(surface))
        fprintf(stderr, "Could not lock surface\n");

    if (surface->format->BitsPerPixel != 32) {
        fprintf(stderr, "Bad bpp for surface: %d\n", surface->format->BitsPerPixel);
        //return;
    }

    if ((surface->w != frame->width) || (surface->h != frame->height)) {
        fprintf(stderr, "Window coords have changed (%d,%d) -> (%d,%d)\n", frame->width, frame->height, surface->w, surface->h);
        return;
    }

    int got_output;

    fflush(stdout);

    /* Get current screen pixels into the frame struct */

    /* If we have access to glReadPixels, use it */

    int size = frame->width * frame->height * 4;
    uint8_t* glpixels = malloc(size);
    //fprintf(stderr, "Call glReadPixels!\n");
    /* TODO: I saw this in some examples before calling glReadPixels: glPixelStorei(GL_PACK_ALIGNMENT, 1); */
    glReadPixels(0, 0, frame->width, frame->height, 0x1908 /* GL_RGBA */, 0x1401 /* GL_UNSIGNED_BYTE */, glpixels);
    //glReadPixels(0, 0, frame->width, frame->height, 0x1907 /* GL_RGB */, 0x1401 /* GL_UNSIGNED_BYTE */, glpixels);
    //fprintf(stderr, "Finish glReadPixels!\n");

    /*
     * Flip image horizontally
     * This is because OpenGL has a different reference point
     * Taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
     * TODO: Could this be done without allocating another array ?
     */
    
    uint8_t* fpixels = malloc(size);
    int pitch = 4 * frame->width;
    for (int line = 0; line < frame->height; line++) {
        int pos = line * pitch;
        for (int p=0; p<pitch; p++) {
            fpixels[pos + p] = glpixels[(size-pos)-pitch + p];
        }
    }

    /* If not, get the pixels from the SDL Surface */

    /* Copy the surface pixels */
    //uint8_t* spixels = malloc(surface->pitch * surface->h);
    //for (int pp=0; pp<surface->pitch * surface->h; pp++)
    //    spixels[pp] = ((pp+3209)*3728) & 0xff;
    //memcpy(spixels, surface->pixels, surface->pitch * surface->h);


    //const uint8_t* orig_plane[4] = { (const uint8_t*)(surface->pixels), 0, 0, 0 };
    //const uint8_t* orig_plane[4] = { (const uint8_t*)(spixels), 0, 0, 0 };
    const uint8_t* orig_plane[4] = { (const uint8_t*)(fpixels), 0, 0, 0 };
    //uint8_t* plane[4] = { (uint8_t*)(frame), (uint8_t*)(frame + ysize), (uint8_t*)(frame + ysize + uvsize), 0 };
    //int orig_stride[4] = {surface->pitch, 0, 0, 0}; // Not sure at all
    int orig_stride[4] = {frame->width * 4, 0, 0, 0}; // Not sure at all
    //int stride[4] = { ystride, uvstride, uvstride, 0 };

    /* Scale the image and copy it into the AVframe */
    //int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
    //            surface->h, plane, stride);
    int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
                frame->height, frame->data, frame->linesize);
    //fprintf(stderr, "ret of scale %d\n", rets);
    //free(spixels);

    frame->pts = fcounter;

    /* encode the image */
    int ret = avcodec_encode_video2(video_st->codec, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }
    if (got_output) {
        pkt.pts = fcounter;
        pkt.dts = fcounter;
        printf("Write frame %ld (size=%5d)\n", fcounter, pkt.size);
        av_write_frame(formatContext, &pkt);
        av_free_packet(&pkt);
    }

    SDL_UnlockSurface_real(surface);
    free(glpixels);
    free(fpixels);
    //free(spixels);
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
            av_write_frame(formatContext, &pkt);
            av_free_packet(&pkt);
        }
    }

    /* add sequence end code to have a real mpeg file */
    //uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    //fwrite(endcode, 1, sizeof(endcode), f);
    //fclose(f);
    
    av_write_trailer(formatContext);
    avio_close(formatContext->pb);
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");
}

