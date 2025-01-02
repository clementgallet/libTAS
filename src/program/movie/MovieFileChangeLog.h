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
#include <QtWidgets/QUndoStack>
#include <list>
#include <vector>
#include <cstdint>

struct Context;

class MovieFileChangeLog : public QUndoStack {
    Q_OBJECT
public:

    /* Prepare a movie file from the context */
    MovieFileChangeLog(Context* c);

    bool undo();
    bool redo();    
private:
    Context* context;

signals:
    void updateChangeLog();
};

#endif
