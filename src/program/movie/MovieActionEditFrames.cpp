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

#include "Context.h"

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, const std::vector<AllInputs>& edited_frames, MovieFileInputs* mi)
{
    first_frame = edit_from;
    last_frame = first_frame + edited_frames.size() - 1;
    movie_inputs = mi;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }

    new_frames = edited_frames;
    setText(QString("Edit frames %1 - %2").arg(first_frame).arg(last_frame));
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, uint64_t edit_to, const std::vector<AllInputs>& edited_frames, MovieFileInputs* mi)
{
    first_frame = edit_from;
    last_frame = edit_to;
    movie_inputs = mi;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }

    new_frames.assign(edited_frames.begin(), edited_frames.begin()+(edit_to-edit_from+1));
    setText(QString("Edit frames %1 - %2").arg(first_frame).arg(last_frame));
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, const AllInputs& edited_frame, MovieFileInputs* mi)
{
    first_frame = edit_from;
    last_frame = edit_from;
    movie_inputs = mi;
    const AllInputs& ai = movie_inputs->getInputs(edit_from);
    old_frames.push_back(ai);
    new_frames.push_back(edited_frame);
    setText(QString("Edit frame %1").arg(edit_from));
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, MovieFileInputs* mi)
{
    first_frame = edit_from;
    last_frame = edit_from;
    movie_inputs = mi;
    const AllInputs& ai = movie_inputs->getInputs(edit_from);
    old_frames.push_back(ai);
    AllInputs clear_ai;
    clear_ai.clear();
    new_frames.push_back(clear_ai);
    setText(QString("Clear frame %1").arg(edit_from));
}

MovieActionEditFrames::MovieActionEditFrames(uint64_t edit_from, uint64_t edit_to, MovieFileInputs* mi)
{
    first_frame = edit_from;
    last_frame = edit_to;
    movie_inputs = mi;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }
    
    setText(QString("Clear frames %1 - %2").arg(first_frame).arg(last_frame));
}

void MovieActionEditFrames::undo() {
    if (first_frame < movie_inputs->context->framecount) return;
    
    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);

    emit movie_inputs->inputsToBeEdited(first_frame, first_frame+old_frames.size()-1);
    std::copy(old_frames.begin(), old_frames.end(), movie_inputs->input_list.begin() + first_frame);
    emit movie_inputs->inputsEdited(first_frame, first_frame+old_frames.size()-1);

    movie_inputs->wasModified();
}

void MovieActionEditFrames::redo() {
    if (first_frame < movie_inputs->context->framecount) return;

    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);

    emit movie_inputs->inputsToBeEdited(first_frame, last_frame);
    if (new_frames.empty()) {
        for (uint64_t i = first_frame; i <= last_frame; i++)
            movie_inputs->input_list[i].clear();
    }
    else {
        std::copy(new_frames.begin(), new_frames.end(), movie_inputs->input_list.begin() + first_frame);
    }
    emit movie_inputs->inputsEdited(first_frame, last_frame);
    movie_inputs->wasModified();
}
