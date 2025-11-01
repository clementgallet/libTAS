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

MovieFileChangeLog::MovieFileChangeLog(Context* c) : QUndoStack(), context(c)
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

void MovieFileChangeLog::push(IMovieAction* action)
{
    int actionCount = count();
    int currentAction = index();
    
    int actionsToRemove = actionCount - currentAction - 1;
    
    /* If we reached the stack limit, the stack size won't change */
    if (actionCount == undoLimit())
        actionsToRemove = 0;
    
    if (actionsToRemove == -1)
        emit historyToBeInserted(actionCount);
    else if (actionsToRemove > 0)
        emit historyToBeRemoved(currentAction+1, actionCount-1);

    QUndoStack::push(action);

    if (actionsToRemove == -1)
        emit historyInserted();
    else if (actionsToRemove > 0)
        emit historyRemoved();
    else
        emit updateChangeLog();
}
