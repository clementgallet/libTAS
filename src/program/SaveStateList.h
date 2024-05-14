/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SAVESTATELIST_H_INCLUDED
#define LIBTAS_SAVESTATELIST_H_INCLUDED

#include <string>
#include <stdint.h>

/* Forward declaration */
class MovieFile;
class SaveState;
struct Context;

namespace SaveStateList {
    
    /* Init savestates and movies */
    void init(Context* context);

    /* Return the savestate from its id */
    SaveState& get(int id);
    
    /* Save state from its id and handle parent */
    int save(int id, Context* context, MovieFile& movie);

    /* Load state from its id */
    int load(int id, Context* context, MovieFile& movie, bool branch, bool inputEditor);

    /* Process after loading state from its id and handle parent */
    int postLoad(int id, Context* context, MovieFile& movie, bool branch, bool inputEditor);

    /* Returns one state id that was performed on that specific frame, or -1 */
    int stateAtFrame(uint64_t frame);

    /* Returns the framecount of the root state, or -1 if already root */
    uint64_t rootStateFramecount();

    /* Returns the previous framecount of the root state */
    uint64_t oldRootStateFramecount();

    /* Returns the nearest state id in current branch before framecount */
    int nearestState(uint64_t framecount);

    /* Save movies on disk when exiting */
    void backupMovies();

}

#endif
