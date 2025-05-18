/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "TimeHolder.h"

#include <vector>
#include <memory> // std::unique_ptr
#include <cstdint>

namespace libtas {

class NutMuxer;

class AVEncoder {
    public:
        /* The constructor sets up the AV dumping into a file.
         * It sets the pipe to an ffmpeg process, and initialize the muxer
         * with the proper screen/sound parameters.
         */
        AVEncoder();

        /* Initialize the muxer. Called by the constructor if parameters are
         * available, or later if parameters are not available yet.
         */
        void initMuxer();

        /* Encode a video and audio frame.
         * @param draw           Is this a draw frame?
         * @param frametime      Length of the frame, used when variable framerate
         */
        void encodeOneFrame(bool draw, TimeHolder frametime);

        /* Close all allocated objects and close the pipe at the end of an av dump
         */
        ~AVEncoder();

        /* Filename of the encode. We use a static array because it can be set
         * very early in the game execution, before objects like std::string
         * has a chance to call its constructor.
         */
        static char dumpfile[4096];

        /* ffmpeg options */
        static char ffmpeg_options[4096];

        static int segment_number;
    private:
        FILE *ffmpeg_pipe = nullptr;
        NutMuxer* nutMuxer = nullptr;

        uint8_t* pixels = nullptr;

        int startup_video_frames = 0;
        std::vector<uint8_t> startup_audio_bytes;

        /* remainder of the number of video frames to send */
        double frame_remainder = 0;
};

extern std::unique_ptr<AVEncoder> avencoder;

}

#endif
