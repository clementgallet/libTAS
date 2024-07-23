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

#include "MovieActionEditFrames.h"
#include "MovieFileInputs.h"

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, const std::vector<AllInputs>& edited_frames, MovieFileInputs* movie_inputs)
{
    first_frame = edit_from;
    last_frame = first_frame + edited_frames.size() - 1;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }

    new_frames = edited_frames;
    description = "Edit frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, uint64_t edit_to, const std::vector<AllInputs>& edited_frames, MovieFileInputs* movie_inputs)
{
    first_frame = edit_from;
    last_frame = edit_to;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }

    new_frames.assign(edited_frames.begin(), edited_frames.begin()+(edit_to-edit_from+1));
    description = "Edit frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, const AllInputs& edited_frame, MovieFileInputs* movie_inputs)
{
    first_frame = edit_from;
    last_frame = edit_from;
    const AllInputs& ai = movie_inputs->getInputs(edit_from);
    old_frames.push_back(ai);
    new_frames.push_back(edited_frame);
    description = "Edit frame ";
    description += std::to_string(edit_from);
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, MovieFileInputs* movie_inputs)
{
    first_frame = edit_from;
    last_frame = edit_from;
    const AllInputs& ai = movie_inputs->getInputs(edit_from);
    old_frames.push_back(ai);
    AllInputs clear_ai;
    clear_ai.clear();
    new_frames.push_back(clear_ai);
    description = "Clear frame ";
    description += std::to_string(edit_from);
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, uint64_t edit_to, MovieFileInputs* movie_inputs)
{
    first_frame = edit_from;
    last_frame = edit_to;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }
    
    description = "Clear frames ";
    description += std::to_string(first_frame);
    description += " - ";
    description += std::to_string(last_frame);
}

void MovieActionEditFrames::undo(MovieFileInputs* movie_inputs) {
    movie_inputs->editInputs(old_frames, first_frame);
}

void MovieActionEditFrames::redo(MovieFileInputs* movie_inputs) {
    if (new_frames.empty()) // clear
        movie_inputs->clearInputs(first_frame, last_frame);
    else
        movie_inputs->editInputs(new_frames, first_frame);
}
