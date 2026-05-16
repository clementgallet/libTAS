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

#include "sdlmutex.h"
#include "sdldynapi.h"

#include "logging.h"

namespace libtas {

    /* Override */ sdl2::SDL_mutex* SDL_CreateMutex(void)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CreateMutex, ());
}

int SDL_LockMutex(sdl2::SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_LockMutex, (mutex));
}

int SDL_TryLockMutex(sdl2::SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_TryLockMutex, (mutex));
}

int SDL_UnlockMutex(sdl2::SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_UnlockMutex, (mutex));
}

void SDL_DestroyMutex(sdl2::SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_DestroyMutex, (mutex));
}

sdl2::SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CreateSemaphore, (initial_value));
}

void SDL_DestroySemaphore(sdl2::SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_DestroySemaphore, (sem));
}

int SDL_SemWait(sdl2::SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemWait, (sem));
}

int SDL_SemTryWait(sdl2::SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemTryWait, (sem));
}

int SDL_SemWaitTimeout(sdl2::SDL_sem * sem, Uint32 ms)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemWaitTimeout, (sem, ms));
}

int SDL_SemPost(sdl2::SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemPost, (sem));
}

Uint32 SDL_SemValue(sdl2::SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemValue, (sem));
}

sdl2::SDL_cond *SDL_CreateCond(void)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CreateCond, ());
}

void SDL_DestroyCond(sdl2::SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_DestroyCond, (cond));
}

int SDL_CondSignal(sdl2::SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondSignal, (cond));
}

int SDL_CondBroadcast(sdl2::SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondBroadcast, (cond));
}

int SDL_CondWait(sdl2::SDL_cond * cond, sdl2::SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondWait, (cond, mutex));
}

int SDL_CondWaitTimeout(sdl2::SDL_cond * cond, sdl2::SDL_mutex * mutex, Uint32 ms)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondWaitTimeout, (cond, mutex, ms));
}


}
