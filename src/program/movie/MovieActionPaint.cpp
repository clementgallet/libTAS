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

#include "MovieActionPaint.h"
#include "MovieFileInputs.h"

MovieActionPaint::MovieActionPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV, MovieFileInputs* movie_inputs) {
    first_frame = start_frame;
    last_frame = end_frame;
    input = si;
    new_value = newV;
    
    for (uint64_t frame = first_frame; frame <= end_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_values.push_back(ai.getInput(si));
    }

    if (first_frame == last_frame) {
        description = "Paint frame ";
        description += std::to_string(start_frame);
    }
    else {
        description = "Paint frames ";
        description += std::to_string(start_frame);
        description += " - ";
        description += std::to_string(end_frame);
    }
}

void MovieActionPaint::undo(MovieFileInputs* movie_inputs) {
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        movie_inputs->paintInput(input, old_values[frame-first_frame], frame, frame);
    }
}

void MovieActionPaint::redo(MovieFileInputs* movie_inputs) {
    movie_inputs->paintInput(input, new_value, first_frame, last_frame);
}
