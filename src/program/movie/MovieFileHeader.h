/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_MOVIEFILEHEADER_H_INCLUDED
#define LIBTAS_MOVIEFILEHEADER_H_INCLUDED

#include "Context.h"
#include <string>

class MovieFileHeader {
public:

    /* Prepare a movie file from the context */
    MovieFileHeader(Context* c);

    /* Import the movie header */
    void load();

    /* Write only the n first frames of input into the movie file. Used for savestate movies */
    void save(uint64_t tot_frames, uint64_t frame_nb);

    /* Get the frame count of the associated savestate if any */
    uint64_t savestateFramecount() const;

    /* Get the movie length from metadata */
    void length(int64_t* sec, int64_t* nsec) const;

    /* Initial framerate values */
    unsigned int framerate_num, framerate_den;

private:
    Context* context;

};

#endif
