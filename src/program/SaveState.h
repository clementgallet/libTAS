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

#ifndef LIBTAS_SAVESTATE_H_INCLUDED
#define LIBTAS_SAVESTATE_H_INCLUDED

#include "movie/MovieFile.h"

#include <string>
#include <memory>
#include <stdint.h>

/* Forward declaration */
struct Context;

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

    /* Id of parent savestate, or -1 if no parent */
    int parent;

    /* Frame count of the savestate */
    uint64_t framecount;

    /* Movie file */
    std::unique_ptr<MovieFile> movie;

    void init(Context* context, int i);

    /* Return the savestate movie path */
    const std::string& getMoviePath() const;

    /* Save state. Return the received message */
    int save(Context* context, const MovieFile& movie);

    /* Load state. Return 0 or error (<0) */
    int load(Context* context, const MovieFile& movie, bool branch, bool inputEditor);

    /* Process after state loading. Return message or error */
    int postLoad(Context* context, MovieFile& movie, bool branch, bool inputEditor);

    /* Save movie on disk when exiting */
    void backupMovie();

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

    /* Savestate messages */
    std::string no_state_msg;
    std::string loading_branch_msg;
    std::string loaded_branch_msg;
    std::string loading_state_msg;
    std::string loaded_state_msg;

    /* Build all savestate messages */
    void buildMessages();

};

#endif
