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

#ifndef LIBTAS_AVDUMPING_H_INCL
#define LIBTAS_AVDUMPING_H_INCL

#ifdef LIBTAS_ENABLE_AVDUMPING

#include <string>
#include <vector>
#include <SDL2/SDL.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

namespace libtas {
class AVEncoder {
    public:
        /* The constructor sets up the AV dumping into a file.
         * This consists mainly of getting the dimensions of the screen,
         * then initialize all objetcs from ffmpeg libraries
         * (context, format, codecs, frames, files, etc.)
         *
         * @param window        Pointer to the SDL_Window* struct that is captured
         * @param video_opengl  Flag indicating if display is done using openGL or
         *                      software SDL rendering
         * @param filename      File where dumping is outputed. File extension
         *                      is important, it is used to guess the file container
         * @param start_frame   Frame when init is done. Does matter if dumping is not
         *                      done from the beginning.
         */
        AVEncoder(SDL_Window* window, bool video_opengl, const char* filename, unsigned long start_frame);

        /* If an error occured during init or encoding, return the error string */
        std::string getErrorMsg();

        /* Encode a video and audio frame.
         * @param fcounter       Frame counter
         * @param draw           Is this a draw frame?
         * @return              -1 if error, 0 if not
         */
        int encodeOneFrame(unsigned long fcounter, bool draw);

        /* Close all allocated objects at the end of an av dump
         */
        ~AVEncoder();

    private:
        int error;
        AVFrame* video_frame;
        AVFrame* audio_frame;
        struct SwsContext *toYUVctx = NULL;
        SwrContext *audio_fmt_ctx = NULL;
        AVOutputFormat *outputFormat = NULL;
        AVFormatContext *formatContext = NULL;
        AVCodecContext *video_codec_context;
        AVCodecContext *audio_codec_context;
        AVStream* video_st;
        AVStream* audio_st;
        std::vector<uint8_t> temp_audio;

        /* Audio samples that could not be encoded because of the
         * audio frame size constraint.
         */
        std::vector<uint8_t> delayed_buffer;

        /* We save the frame when the dumping begins */
        unsigned long start_frame = -1;

        /* The accumulated number of audio samples */
        uint64_t accum_samples;

        /* Send audio frame */
        int send_audio_frame();

        /* Receive packets and send them to the muxer */
        int receive_packet(AVCodecContext *codec_context, AVStream* st);

};
}

#endif
#endif
