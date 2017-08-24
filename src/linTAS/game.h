/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_GAME_H_INCLUDED
#define LINTAS_GAME_H_INCLUDED

#include "Context.h"
#include "MovieFile.h"

/* TODO: I really don't like this extern, but let's use it for now.
 * The thing is that we need the UI to be able to save the movie.
 * In the scenario where the UI sends a command for the game-handling thread to
 * save, it can lead to a problematic case, because if the game is frozen, the
 * game-handling thread is waiting forever for the frame boundary and never
 * perform its actions, including the movie saving. It is safer for the UI
 * thread to perform the save itself, because it is less likely to be stuck.
 */
extern MovieFile movie;

void launchGame(Context* context);

#endif
