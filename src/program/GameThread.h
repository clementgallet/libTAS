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
    void launch(Context *context);    

    /* Auto-detect the local directory containing the bundle libraries shipped
     * with the game, so that in most cases users don't have to specify it. */
    void detect_game_libraries(Context *context);
    
    /* Set all environment variables before launching the game process */
    void set_env_variables(Context *context, int gameArch);
    
    /* Detect and returns the game executable arch */
    int detect_arch(Context *context);

    /* Build the list of all arguments to be passed to `sh` for running the game */
    std::list<std::string> build_arg_list(Context *context, int gameArch);
}

#endif
