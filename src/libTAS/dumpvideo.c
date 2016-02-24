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

uint8_t* glpixels;
uint8_t* glpixels_flip;

int useGL;

int openVideoDump(void* window, int video_opengl) {

    /* Get information about the current screen */
    int width, height;
    SDL_GL_GetDrawableSize_real(window, &width, &height);

    /* If the game uses openGL, the method to capture the screen will be different */
    useGL = video_opengl;

    if (useGL) {
        if (!glReadPixels_real) {
            fprintf(stderr, "glReadPixels is needed but is not exported by the game.\n");
            fprintf(stderr, "Importing it using SDL_GL_GetProcAddress.\n");
            glReadPixels_real = SDL_GL_GetProcAddress_real("glReadPixels");
            if (!glReadPixels_real) {
                fprintf(stderr, "Could not load function glReadPixels\n");
                return 1;
            }
        }

        /* Initialize buffers for screen pixels */
        int size = width * height * 4;
        glpixels = malloc(size);
        glpixels_flip = malloc(size);
    }

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
        return 1;
    }

    // define AVFormatContext
    // lallocate the output media context
    formatContext = avformat_alloc_context();
    //avformat_alloc_output_context2(&formatContext, NULL, NULL, filename);
//  avformat_alloc_output_context(&formatContext, outputFormat, NULL, NULL);
    if (!formatContext) {
        fprintf(stderr, "Memory error\n");
        return 1;
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
        return 1;
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
    video_st->codec->time_base = (AVRational){1,60};
    video_st->codec->gop_size = 10; /* emit one intra frame every ten frames */
    video_st->codec->max_b_frames = 1;
    video_st->codec->pix_fmt = AV_PIX_FMT_YUV420P;

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;


    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(video_st->codec->priv_data, "preset", "slow", 0);

    if (avcodec_open2(video_st->codec, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return 1;
    }
    
    
    fprintf(stderr, "Alloc frame\n");
    
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return 1;
    }
    frame->format = video_st->codec->pix_fmt;
    frame->width  = video_st->codec->width;
    frame->height = video_st->codec->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        return 1;
    }

    /* Just in case dimensions are not a multiple of 2 */
    int yuvwidth  = RNDTO2 ( frame->width );
    int yuvheight = RNDTO2 ( frame->height );

    /* Initialize swscale for pixel format conversion */
    toYUVctx = sws_getContext(frame->width, frame->height,  
                              PIX_FMT_RGBA,
                              yuvwidth, yuvheight, 
                              PIX_FMT_YUV420P,
                              SWS_LANCZOS | SWS_ACCURATE_RND, NULL,NULL,NULL);

    if (toYUVctx == NULL) {
        fprintf(stderr, "Could not allocate swscale context\n");
        return 1;
    }


    av_dump_format(formatContext, 0, filename, 1);
    
    /* Set up output file */
    avio_open(&formatContext->pb, filename, AVIO_FLAG_WRITE);

    /* Write header */
    int rh = avformat_write_header(formatContext, NULL);
    if (rh < 0) {
        fprintf(stderr, "Could not write header!\n");
        return 1;
    }

    return 0;
}

void encodeOneFrame(unsigned long fcounter, void* window) {

    /* Initialize AVPacket */
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    SDL_Surface* surface = NULL;

    const uint8_t* orig_plane[4] = {0};
    int orig_stride[4] = {0};

    if (useGL) {

        /* We access to the image pixels directly using glReadPixels */
        int size = frame->width * frame->height * 4;
        /* TODO: I saw this in some examples before calling glReadPixels: glPixelStorei(GL_PACK_ALIGNMENT, 1); */
        glReadPixels_real(0, 0, frame->width, frame->height, 0x1908 /* GL_RGBA */, 0x1401 /* GL_UNSIGNED_BYTE */, glpixels);

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

        /* Get surface from window */
        surface = SDL_GetWindowSurface_real(window);

        /* We must lock the surface before accessing the raw pixels */
        if (0 != SDL_LockSurface_real(surface))
            fprintf(stderr, "Could not lock surface\n");

        /* Currently only supporting ARGB (32 bpp) pixel format */
        if (surface->format->BitsPerPixel != 32) {
            fprintf(stderr, "Bad bpp for surface: %d\n", surface->format->BitsPerPixel);
            return;
        }

        /* Checking for a size modification */
        if ((surface->w != frame->width) || (surface->h != frame->height)) {
            fprintf(stderr, "Window coords have changed (%d,%d) -> (%d,%d)\n", frame->width, frame->height, surface->w, surface->h);
            return;
        }

        orig_plane[0] = (const uint8_t*)(surface->pixels);
        orig_stride[0] = surface->pitch;
    }


    /* Change pixel format to YUV420p and copy it into the AVframe */
    int rets = sws_scale(toYUVctx, orig_plane, orig_stride, 0, 
                frame->height, frame->data, frame->linesize);
    if (rets != frame->height) {
        fprintf(stderr, "We only could scale %d rows\n", rets);
        return;
    }

    frame->pts = fcounter;

    /* encode the image */
    int got_output;
    int ret = avcodec_encode_video2(video_st->codec, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        return;
    }
    if (got_output) {
        /* TODO: check these values */
        pkt.pts = fcounter;
        pkt.dts = fcounter;
        //printf("Write frame %ld (size=%5d)\n", fcounter, pkt.size);
        av_write_frame(formatContext, &pkt);
        av_free_packet(&pkt);
    }

    if (!useGL) {
        /* Unlock surface */
        SDL_UnlockSurface_real(surface);
    }
}


void closeVideoDump() {
    /* Encode the remaining frames */
    for (int got_output = 1; got_output;) {
        int ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            //printf("Write frame %3d (size=%5d)\n", -1, pkt.size);
            av_write_frame(formatContext, &pkt);
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
    avformat_free_context(formatContext);
    sws_freeContext(toYUVctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}

