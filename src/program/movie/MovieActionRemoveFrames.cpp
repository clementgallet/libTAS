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

#include "Context.h"

MovieActionRemoveFrames::MovieActionRemoveFrames(uint64_t remove_from, uint64_t remove_to, MovieFileInputs* mi)
{
    first_frame = remove_from;
    last_frame = remove_to;
    movie_inputs = mi;
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }
    
    setText(QString("Remove frames %1 - %2").arg(first_frame).arg(last_frame));
}

void MovieActionRemoveFrames::storeOldInputs()
{
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_frames.push_back(ai);
    }
}

void MovieActionRemoveFrames::undo() {
    if (first_frame < movie_inputs->context->framecount) return;

    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);
    emit movie_inputs->inputsToBeInserted(first_frame, first_frame+old_frames.size()-1);
    movie_inputs->input_list.insert(movie_inputs->input_list.begin() + first_frame, old_frames.begin(), old_frames.end());
    emit movie_inputs->inputsInserted(first_frame, first_frame+old_frames.size()-1);
    movie_inputs->wasModified();
}

void MovieActionRemoveFrames::redo() {
    if (first_frame < movie_inputs->context->framecount) return;

    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);
    
    emit movie_inputs->inputsToBeRemoved(first_frame, last_frame);

    if ((last_frame + 1) == movie_inputs->input_list.size())
        movie_inputs->input_list.resize(first_frame);
    else
        movie_inputs->input_list.erase(movie_inputs->input_list.begin() + first_frame, movie_inputs->input_list.begin() + last_frame + 1);

    emit movie_inputs->inputsRemoved(first_frame, last_frame);
    movie_inputs->wasModified();
}
