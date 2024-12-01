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

#ifndef LIBTAS_MOVIEFILEHEADER_H_INCLUDED
#define LIBTAS_MOVIEFILEHEADER_H_INCLUDED

#include <string>
#include <stdint.h>

struct Context;

class MovieFileHeader {
public:

    /* Prepare a movie file from the context */
    MovieFileHeader(Context* c);

    /* Clear */
    void clear();

    /* Import the movie header */
    void load();

    /* Only load parameters used for savestates */
    void loadSavestate();

    /* Write only the n first frames of input into the movie file. Used for savestate movies */
    void save(uint64_t tot_frames, uint64_t frame_nb);

    /* Initial framerate values */
    unsigned int framerate_num, framerate_den;

    /* Savestate framecount */
    uint64_t savestate_framecount;

    /* Movie length, only used for savestate movies */
    int64_t length_sec, length_nsec;

    /* Skip loading some settings so that users can change settings */
    bool skipLoadSettings = false;

    /* Authors of the movie */
    std::string authors;

    /* MD5 hash of the game executable that is stored in the movie */
    std::string md5_movie;
    
    /* Store if movie has variable framerate */
    bool variable_framerate;

private:
    Context* context;

};

#endif
