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

#include "MovieFileChangeLog.h"
#include "MovieFileInputs.h"
#include "MovieActionEditFrames.h"
#include "MovieActionInsertFrames.h"
#include "MovieActionPaint.h"
#include "MovieActionRemoveFrames.h"

#include "Context.h"
#include "../shared/inputs/AllInputs.h"

MovieFileChangeLog::MovieFileChangeLog(Context* c, MovieFileInputs* mi) : context(c), movie_inputs(mi)
{
    max_steps = 100;
    is_recording = true;
    history_index = 0;
}

void MovieFileChangeLog::clear()
{
    emit beginResetHistory();
    history.clear();
    history_index = 0;
    emit endResetHistory();
}

void MovieFileChangeLog::truncateLog(int frame)
{
    emit beginRemoveHistory(frame, history.size()-1);
    history.resize(frame);
    emit endRemoveHistory();
    if (history_index > history.size())
        history_index = history.size();
}

bool MovieFileChangeLog::canUndo()
{
    return history_index > 0;
}

bool MovieFileChangeLog::undo()
{
    if (!canUndo())
        return false;
    
    IMovieAction* action = (*std::next(history.begin(), history_index-1));
    if (action->first_frame < context->framecount)
        return false;
    
    bool was_recording = is_recording;
    is_recording = false;
    
    action->undo(movie_inputs);
    history_index--;
    
    emit changeHistory(history_index);

    is_recording = was_recording;
    return true;
}

bool MovieFileChangeLog::canRedo()
{
    return history_index < history.size();
}

bool MovieFileChangeLog::redo()
{
    if (!canRedo())
        return false;
    
    IMovieAction* action = (*std::next(history.begin(), history_index));
    if (action->first_frame < context->framecount)
        return false;

    bool was_recording = is_recording;
    is_recording = false;
    
    history_index++;
    action->redo(movie_inputs);

    emit changeHistory(history_index-1);

    is_recording = was_recording;
    return true;
}

void MovieFileChangeLog::registerAction()
{
    if (history_index < history.size()) {
        truncateLog(history_index);
    }
    
    if (history.size() <= max_steps)
        history_index++;
    else
        history.pop_front(); // TODO
}

void MovieFileChangeLog::registerPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionPaint(start_frame, end_frame, si, newV, movie_inputs));
    emit endAddHistory();
}

void MovieFileChangeLog::registerClearFrames(uint64_t start_frame, uint64_t end_frame)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionEditFrames(start_frame, end_frame, movie_inputs));
    emit endAddHistory();
}

void MovieFileChangeLog::registerEditFrame(uint64_t edit_from, const AllInputs& edited_frame)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionEditFrames(edit_from, edited_frame, movie_inputs));
    emit endAddHistory();
}

void MovieFileChangeLog::registerEditFrames(uint64_t edit_from, const std::vector<AllInputs>& edited_frames)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionEditFrames(edit_from, edited_frames, movie_inputs));
    emit endAddHistory();
}

void MovieFileChangeLog::registerEditFrames(uint64_t edit_from, uint64_t edit_to, const std::vector<AllInputs>& edited_frames)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionEditFrames(edit_from, edit_to, edited_frames, movie_inputs));
    emit endAddHistory();
}

void MovieFileChangeLog::registerInsertFrame(uint64_t insert_from, const AllInputs& inserted_frame)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionInsertFrames(insert_from, inserted_frame));
    emit endAddHistory();
}

void MovieFileChangeLog::registerInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames)
{
    if (!is_recording) return;
    
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionInsertFrames(insert_from, inserted_frames));
    emit endAddHistory();
}

void MovieFileChangeLog::registerInsertFrames(uint64_t insert_from, int count)
{
    if (!is_recording) return;
    
    if (count <= 0)
        return;
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionInsertFrames(insert_from, count));
    emit endAddHistory();
}

void MovieFileChangeLog::registerRemoveFrames(uint64_t remove_from, uint64_t remove_to)
{
    if (!is_recording) return;
    
    if (remove_to < remove_from)
        return;
    registerAction();
    emit beginAddHistory(history.size());
    history.push_back(new MovieActionRemoveFrames(remove_from, remove_to, movie_inputs));
    emit endAddHistory();
}
