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
    
    for (uint64_t frame = first_frame; frame <= end_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_values.push_back(ai.getInput(si));
    }

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
    
    for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
        const AllInputs& ai = movie_inputs->getInputs(frame);
        old_values.push_back(ai.getInput(si));
    }

    if (first_frame == last_frame) {
        setText(QString("Paint frame %1").arg(first_frame));
    }
    else {
        setText(QString("Paint frames %1 - %2").arg(first_frame).arg(last_frame));
    }
}

void MovieActionPaint::undo() {
    if (first_frame < movie_inputs->context->framecount) return;

    for (size_t i = 0; i < old_values.size(); i++)
        movie_inputs->queueInput(first_frame+i, input, old_values[i], false);

    /* Process paint actions synchronously, because the order of undo/redo 
     * actions must be enforced */
    movie_inputs->processPendingInputs();
}

void MovieActionPaint::redo() {
    if (first_frame < movie_inputs->context->framecount) return;

    if (new_values.empty()) {
        for (uint64_t i = first_frame; i <= last_frame; i++)
            movie_inputs->queueInput(i, input, new_value, false);
    }
    else {
        for (size_t i = 0; i < new_values.size(); i++)
            movie_inputs->queueInput(first_frame+i, input, new_values[i], false);
    }

    /* Process paint actions synchronously, because the order of undo/redo 
     * actions must be enforced */
    movie_inputs->processPendingInputs();
}
