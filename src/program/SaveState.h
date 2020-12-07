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

#ifndef LIBTAS_SAVESTATE_H_INCLUDED
#define LIBTAS_SAVESTATE_H_INCLUDED

#include "Context.h"
#include "MovieFile.h"
#include <string>

class SaveState {
public:
    
    SaveState () : parent(-1) {}
    
    /* List of error codes */
    enum Error {
        ENOSTATEMOVIEPREFIX = -1, // No state but movie with matching prefix
        ENOSTATE = -2, // No state
        ENOMOVIE = -3, // Could not moad movie
        EINPUTMISMATCH = -4, // Mistmatch inputs
        ENOLOAD = -5, // State loading failed
    };

    /* Savestate number */
    int id;

    /* Is backtrack savestate */
    bool is_backtrack;

    /* Id of parent savestate, or -1 if no parent */
    int parent;

    /* Return the savestate movie path */
    const std::string& getMoviePath();

    /* Save state. Return the received message */
    int save(Context* context, MovieFile& movie);

    /* Load state. Return 0 or error (<0) */
    int load(Context* context, MovieFile& movie, bool branch);

    /* Process after state loading. Return message or error */
    int postLoad(Context* context, MovieFile& movie, bool branch);

private:
    /* Savestate path */
    std::string path;

    /* Savestate pagemap path */
    std::string pagemap_path;

    /* Savestate pages path */
    std::string pages_path;

    /* Savestate movie path */
    std::string movie_path;

    /* Build all savestate paths */
    void buildPaths(Context* context);

    /* Savestate saving message */
    std::string saving_msg;
    
    /* Savestate saved message */
    std::string saved_msg;

    /* No savestate message */
    std::string no_state_msg;

    /* Savestate loading message */
    std::string loading_msg;

    /* Savestate loaded message */
    std::string loaded_msg;

    /* Build all savestate messages */
    void buildMessages(Context* context);

};

#endif
