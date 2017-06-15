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

#include "sdltimer.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "hook.h"

namespace libtas {

namespace orig {
    static SDL_TimerID (*SDL_AddTimer)(Uint32 interval, SDL_NewTimerCallback callback, void *param);
    static SDL_bool (*SDL_RemoveTimer)(SDL_TimerID id);
}

/* Override */ SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param)
{
    debuglog(LCF_TIMEFUNC | LCF_SDL | LCF_TODO, "Add SDL Timer with call after ", interval, " ms");
    return orig::SDL_AddTimer(interval, callback, param);
}

/* Override */ SDL_bool SDL_RemoveTimer(SDL_TimerID id)
{
    debuglog(LCF_TIMEFUNC | LCF_SDL | LCF_TODO, "Remove SDL Timer.");
    return orig::SDL_RemoveTimer(id);
}

void link_sdltimer(void)
{
    LINK_NAMESPACE_SDLX(SDL_AddTimer);
    LINK_NAMESPACE_SDLX(SDL_RemoveTimer);
    /* TODO: Add SDL 1.2 SetTimer */
}

}
