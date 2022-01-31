/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_GAMETHREAD_H_INCLUDED
#define LIBTAS_GAMETHREAD_H_INCLUDED

#include "Context.h"

namespace GameThread {
    /* Set the different environment variables, then start the game executable with
     * our library to be injected using the LD_PRELOAD trick.
     * Because this function eventually calls execl, it does not return.
     * So, it is called from a child process using fork().
     */
    void launch(Context *context, int sock);
}

#endif
