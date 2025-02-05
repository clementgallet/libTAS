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

#include "Context.h"

MovieActionPaint::MovieActionPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV, MovieFileInputs* mi)
{
    first_frame = start_frame;
    last_frame = end_frame;
    movie_inputs = mi;
    input = si;
    new_value = newV;
    
    if (first_frame == last_frame) {
        setText(QString("Paint frame %1").arg(first_frame));
    }
    else {
        setText(QString("Paint frames %1 - %2").arg(first_frame).arg(last_frame));
    }
}

MovieActionPaint::MovieActionPaint(uint64_t start_frame, SingleInput si, const std::vector<int>& newV, MovieFileInputs* mi)
{
    first_frame = start_frame;
    last_frame = start_frame + newV.size() - 1;
    movie_inputs = mi;
    input = si;
    new_values = newV;
    
    if (first_frame == last_frame) {
        setText(QString("Paint frame %1").arg(first_frame));
    }
    else {
        setText(QString("Paint frames %1 - %2").arg(first_frame).arg(last_frame));
    }
}

void MovieActionPaint::storeOldInputs()
{
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_values.push_back(ai.getInput(input));
    }
}

void MovieActionPaint::undo() {
    if (first_frame < movie_inputs->context->framecount) return;

    if (last_frame >= movie_inputs->input_list.size()) return;
    
    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);

    emit movie_inputs->inputsToBeEdited(first_frame, last_frame);
    for (size_t i = 0; i < old_values.size(); i++) {
        AllInputs& ai = movie_inputs->input_list[first_frame+i];
        ai.setInput(input, old_values[i]);
    }
    emit movie_inputs->inputsEdited(first_frame, last_frame);
    movie_inputs->wasModified();
}

void MovieActionPaint::redo() {
    if (first_frame < movie_inputs->context->framecount) return;

    if (last_frame >= movie_inputs->input_list.size()) return;
    
    std::unique_lock<std::mutex> lock(movie_inputs->input_list_mutex);

    emit movie_inputs->inputsToBeEdited(first_frame, last_frame);
    if (new_values.empty()) {
        for (uint64_t i = first_frame; i <= last_frame; i++) {
            AllInputs& ai = movie_inputs->input_list[i];
            ai.setInput(input, new_value);
        }
    }
    else {
        for (size_t i = 0; i < new_values.size(); i++) {
            AllInputs& ai = movie_inputs->input_list[first_frame+i];
            ai.setInput(input, new_values[i]);
        }
    }
    emit movie_inputs->inputsEdited(first_frame, last_frame);
    movie_inputs->wasModified();
}
