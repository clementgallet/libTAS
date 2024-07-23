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

#ifndef LIBTAS_MOVIEFILECHANGELOG_H_INCLUDED
#define LIBTAS_MOVIEFILECHANGELOG_H_INCLUDED

#include "../shared/inputs/AllInputs.h"

#include "IMovieAction.h"

#include <QtCore/QObject>
#include <list>
#include <vector>
#include <cstdint>

struct Context;
class MovieFileInputs;

class MovieFileChangeLog : public QObject {
    Q_OBJECT
public:
    /* Prepare a movie file from the context */
    MovieFileChangeLog(Context* c, MovieFileInputs* mi);

    void clear();
    void truncateLog(int frame);
    bool canUndo();
    bool undo();
    bool canRedo();
    bool redo();
    void registerAction();
    void registerPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV);
    void registerClearFrames(uint64_t start_frame, uint64_t end_frame);
    void registerEditFrame(uint64_t edit_from, const AllInputs& edited_frame);
    void registerEditFrames(uint64_t edit_from, const std::vector<AllInputs>& edited_frames);
    void registerEditFrames(uint64_t edit_from, uint64_t edit_to, const std::vector<AllInputs>& edited_frames);
    void registerInsertFrame(uint64_t insert_from, const AllInputs& inserted_frame);
    void registerInsertFrames(uint64_t insert_from, const std::vector<AllInputs>& inserted_frames);
    void registerInsertFrames(uint64_t insert_from, int count);
    void registerRemoveFrames(uint64_t remove_from, uint64_t remove_to);
    
    std::list<IMovieAction*> history;
    
    /* Index to the next inserted action */
    unsigned int history_index;

private:
    Context* context;
    unsigned int max_steps;

    MovieFileInputs* movie_inputs;
    bool is_recording;

signals:
    void beginAddHistory(int frame);
    void endAddHistory();
    void beginRemoveHistory(int first_frame, int last_frame);
    void endRemoveHistory();
    void changeHistory(int frame);

};

#endif
