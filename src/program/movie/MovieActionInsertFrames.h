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

#ifndef LIBTAS_MOVIEACTIONINSERTFRAMES_H_INCLUDED
#define LIBTAS_MOVIEACTIONINSERTFRAMES_H_INCLUDED

#include "IMovieAction.h"
#include "../shared/inputs/AllInputs.h"

#include <vector>
#include <cstdint>

class MovieFileInputs;

class MovieActionInsertFrames : public IMovieAction {
public:
    MovieActionInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames);
    MovieActionInsertFrames(uint64_t insert_from, int count);
    MovieActionInsertFrames(uint64_t insert_from, const AllInputs& inserted_frame);

    void undo(MovieFileInputs* movie_inputs);
    void redo(MovieFileInputs* movie_inputs);
    
private:
    std::vector<AllInputs> new_frames;
};

#endif
