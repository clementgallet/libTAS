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

#ifndef LIBTAS_AVDUMPING_H_INCL
#define LIBTAS_AVDUMPING_H_INCL

#include "NutMuxer.h"
#include <vector>
#include <SDL2/SDL.h>
#include <memory> // std::unique_ptr

namespace libtas {
class AVEncoder {
    public:
        /* The constructor sets up the AV dumping into a file.
         * This consists mainly of getting the dimensions of the screen,
         * then starting a pipe into an ffmpeg process
         *
         * @param window        Pointer to the SDL_Window* struct that is captured
         */
        AVEncoder(SDL_Window* window);

        /* Encode a video and audio frame.
         * @param draw           Is this a draw frame?
         * @return              -1 if error, 0 if not
         */
        int encodeOneFrame(bool draw);

        /* Close all allocated objects at the end of an av dump
         */
        ~AVEncoder();

        /* Filename of the encode. We use a static array because it can be set
         * very early in the game execution, before objects like std::string
         * has a chance to call its constructor.
         */
        static char dumpfile[4096];

        /* ffmpeg options */
        static char ffmpeg_options[4096];

    private:
        FILE *ffmpeg_pipe;
        NutMuxer* nutMuxer;

};

extern std::unique_ptr<AVEncoder> avencoder;

}

#endif
