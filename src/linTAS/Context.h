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

#ifndef LINTAS_CONTEXT_H_INCLUDED
#define LINTAS_CONTEXT_H_INCLUDED

#include <string>
#include "KeyMapping.h"
#include "../shared/tasflags.h"

struct Context {
    /* Execution status */
    enum RunStatus {
        INACTIVE,
        STARTING,
        ACTIVE,
        QUITTING
    };
    RunStatus status;

    /* frame count */
    unsigned long int framecount = 0;

    /* tas flags */
    TasFlags tasflags;

    /* key mapping */
    KeyMapping km;

    /* was the tasflags modified */
    bool tasflags_modified;

    /* Absolute path of libTAS.so */
    std::string libtaspath;

    /* Absolute path of the game executable */
    std::string gamepath;

    /* Arguments passed to the game */
    std::string gameargs;

    /* Absolute path of the movie file */
    std::string moviefile;

    /* Absolute path of the dump file */
    std::string dumpfile;

    /* Path of the libraries used by the game */
    std::string libdir;

    /* Path where the game needs to run */
    std::string rundir;
};

#endif
