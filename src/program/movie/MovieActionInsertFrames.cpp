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

#include "MovieActionInsertFrames.h"
#include "MovieFileInputs.h"

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames)
{
    first_frame = insert_from;
    last_frame = first_frame + inserted_frames.size() - 1;
    new_frames = inserted_frames;
    description = "Insert frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, int count)
{
    first_frame = insert_from;
    last_frame = first_frame + count - 1;
    new_frames.clear();
    description = "Insert frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, const AllInputs& inserted_frame)
{
    first_frame = insert_from;
    last_frame = insert_from;
    new_frames.push_back(inserted_frame);
    description = "Insert at frame ";
    description += std::to_string(insert_from);
}

void MovieActionInsertFrames::undo(MovieFileInputs* movie_inputs) {
    movie_inputs->deleteInputs(first_frame, last_frame-first_frame+1);
}

void MovieActionInsertFrames::redo(MovieFileInputs* movie_inputs) {
    if (new_frames.empty())
        movie_inputs->insertInputsBefore(first_frame, last_frame-first_frame+1);
    else
        movie_inputs->insertInputsBefore(new_frames, first_frame);
}
