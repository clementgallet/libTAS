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

MovieFileChangeLog::MovieFileChangeLog(Context* c, MovieFileInputs* mi) : QUndoStack(), context(c), movie_inputs(mi)
{
    clear();
    setUndoLimit(100);
}

bool MovieFileChangeLog::undo()
{
    if (!canUndo())
        return false;
    
    const IMovieAction* action = dynamic_cast<const IMovieAction*>(command(index()-1));
    if (action->first_frame < context->framecount)
        return false;
    
    QUndoStack::undo();
    emit updateChangeLog();
    return true;
}

bool MovieFileChangeLog::redo()
{
    if (!canRedo())
        return false;

    const IMovieAction* action = dynamic_cast<const IMovieAction*>(command(index()));
    if (action->first_frame < context->framecount)
        return false;
    
    QUndoStack::redo();
    emit updateChangeLog();
    return true;
}

void MovieFileChangeLog::registerPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV)
{
    push(new MovieActionPaint(start_frame, end_frame, si, newV, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerPaint(uint64_t start_frame, SingleInput si, const std::vector<int>& newV)
{
    push(new MovieActionPaint(start_frame, si, newV, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerClearFrames(uint64_t start_frame, uint64_t end_frame)
{
    push(new MovieActionEditFrames(start_frame, end_frame, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerEditFrame(uint64_t edit_from, const AllInputs& edited_frame)
{
    push(new MovieActionEditFrames(edit_from, edited_frame, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerEditFrames(uint64_t edit_from, const std::vector<AllInputs>& edited_frames)
{
    push(new MovieActionEditFrames(edit_from, edited_frames, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerEditFrames(uint64_t edit_from, uint64_t edit_to, const std::vector<AllInputs>& edited_frames)
{
    push(new MovieActionEditFrames(edit_from, edit_to, edited_frames, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerInsertFrame(uint64_t insert_from, const AllInputs& inserted_frame)
{
    push(new MovieActionInsertFrames(insert_from, inserted_frame, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames)
{
    push(new MovieActionInsertFrames(insert_from, inserted_frames, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerInsertFrames(uint64_t insert_from, int count)
{
    if (count <= 0)
        return;
    push(new MovieActionInsertFrames(insert_from, count, movie_inputs));
    emit updateChangeLog();
}

void MovieFileChangeLog::registerRemoveFrames(uint64_t remove_from, uint64_t remove_to)
{
    if (remove_to < remove_from)
        return;
    push(new MovieActionRemoveFrames(remove_from, remove_to, movie_inputs));
    emit updateChangeLog();
}
