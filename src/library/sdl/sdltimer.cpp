/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdltimer.h"
#include "sdldynapi.h"

#include "logging.h"
#include "DeterministicTimer.h"

namespace libtas {

/* Override */ sdl2::SDL_TimerID SDL_AddTimer(Uint32 interval, sdl2::SDL_NewTimerCallback callback, void *param)
{
    LOG(LL_TRACE, LCF_TIMERS | LCF_SDL | LCF_TODO, "Add SDL Timer with call after %d ms", interval);
    return ORIG_SDL2_CALL(SDL_AddTimer, (interval, callback, param));
}

/* Override */ SDL_bool SDL_RemoveTimer(sdl2::SDL_TimerID id)
{
    LOG(LL_TRACE, LCF_TIMERS | LCF_SDL | LCF_TODO, "Remove SDL Timer.");
    return ORIG_SDL2_CALL(SDL_RemoveTimer, (id));
}

}
