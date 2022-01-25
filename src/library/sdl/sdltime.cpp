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

#include "sdltime.h"
#include "../logging.h"
#include "../DeterministicTimer.h"
#include "../hook.h"

namespace libtas {

/* Override */ Uint32 SDL_GetTicks(void)
{
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETTICKS);
    Uint32 msec = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    debuglogstdio(LCF_SDL | LCF_TIMEGET, "%s call - returning %d", __func__, msec);

    return msec;
}

/* Override */ Uint64 SDL_GetTicks64(void)
{
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETTICKS);
    Uint64 msec = ts.tv_sec*1000ull + ts.tv_nsec/1000000ull;
    debuglogstdio(LCF_SDL | LCF_TIMEGET, "%s call - returning %llu", __func__, msec);

    return msec;
}

/* Override */ Uint64 SDL_GetPerformanceFrequency(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_TIMEGET);
    return 1000000000;
}

/* Override */ Uint64 SDL_GetPerformanceCounter(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_TIMEGET);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER);
    Uint64 counter = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    debuglogstdio(LCF_SDL | LCF_TIMEGET, "  returning %" PRIu64, counter);
    return counter;
}

}
