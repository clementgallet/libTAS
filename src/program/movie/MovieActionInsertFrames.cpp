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

#include "Context.h"

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames, MovieFileInputs* mi)
{
    first_frame = insert_from;
    last_frame = first_frame + inserted_frames.size() - 1;
    movie_inputs = mi;
    new_frames = inserted_frames;
    setText(QString("Insert frames %1 - %2").arg(first_frame).arg(last_frame));
}

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, int count, MovieFileInputs* mi)
{
    first_frame = insert_from;
    last_frame = first_frame + count - 1;
    movie_inputs = mi;
    new_frames.clear();
    setText(QString("Insert frames %1 - %2").arg(first_frame).arg(last_frame));
}

MovieActionInsertFrames::MovieActionInsertFrames(uint64_t insert_from, const AllInputs& inserted_frame, MovieFileInputs* mi)
{
    first_frame = insert_from;
    last_frame = insert_from;
    movie_inputs = mi;
    new_frames.push_back(inserted_frame);
    setText(QString("Insert at frame %1").arg(insert_from));
}

void MovieActionInsertFrames::undo() {
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

void MovieActionInsertFrames::redo() {
    if (first_frame < movie_inputs->context->framecount) return;

    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);

    emit movie_inputs->inputsToBeInserted(first_frame, last_frame);
    
    if (new_frames.empty()) {
        AllInputs ai;
        ai.clear();
        movie_inputs->input_list.insert(movie_inputs->input_list.begin() + first_frame, last_frame-first_frame+1, ai);
    }
    else
        movie_inputs->input_list.insert(movie_inputs->input_list.begin() + first_frame, new_frames.begin(), new_frames.end());
    
    emit movie_inputs->inputsInserted(first_frame, last_frame);
    movie_inputs->wasModified();
}
