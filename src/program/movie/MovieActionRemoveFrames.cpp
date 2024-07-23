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

#include "MovieActionRemoveFrames.h"
#include "MovieFileInputs.h"

MovieActionRemoveFrames::MovieActionRemoveFrames(uint64_t remove_from, uint64_t remove_to, MovieFileInputs* movie_inputs) {
    first_frame = remove_from;
    last_frame = remove_to;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }
    
    description = "Remove frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

void MovieActionRemoveFrames::undo(MovieFileInputs* movie_inputs) {
    movie_inputs->insertInputsBefore(old_frames, first_frame);
}

void MovieActionRemoveFrames::redo(MovieFileInputs* movie_inputs) {
    movie_inputs->deleteInputs(first_frame, last_frame-first_frame+1);
}
