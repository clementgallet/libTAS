/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "logging.h"
#include "hook.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateMutex)
DECLARE_ORIG_POINTER(SDL_LockMutex)
DECLARE_ORIG_POINTER(SDL_TryLockMutex)
DECLARE_ORIG_POINTER(SDL_UnlockMutex)
DECLARE_ORIG_POINTER(SDL_DestroyMutex)
DECLARE_ORIG_POINTER(SDL_CreateSemaphore)
DECLARE_ORIG_POINTER(SDL_DestroySemaphore)
DECLARE_ORIG_POINTER(SDL_SemWait)
DECLARE_ORIG_POINTER(SDL_SemTryWait)
DECLARE_ORIG_POINTER(SDL_SemWaitTimeout)
DECLARE_ORIG_POINTER(SDL_SemPost)
DECLARE_ORIG_POINTER(SDL_SemValue)
DECLARE_ORIG_POINTER(SDL_CreateCond)
DECLARE_ORIG_POINTER(SDL_DestroyCond)
DECLARE_ORIG_POINTER(SDL_CondSignal)
DECLARE_ORIG_POINTER(SDL_CondBroadcast)
DECLARE_ORIG_POINTER(SDL_CondWait)
DECLARE_ORIG_POINTER(SDL_CondWaitTimeout)

/* Override */ SDL_mutex* SDL_CreateMutex(void)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CreateMutex);
    return orig::SDL_CreateMutex();
}

int SDL_LockMutex(SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_LockMutex);
    return orig::SDL_LockMutex(mutex);
}

int SDL_TryLockMutex(SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_TryLockMutex);
    return orig::SDL_TryLockMutex(mutex);
}

int SDL_UnlockMutex(SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_UnlockMutex);
    return orig::SDL_UnlockMutex(mutex);
}

void SDL_DestroyMutex(SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_DestroyMutex);
    return orig::SDL_DestroyMutex(mutex);
}

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CreateSemaphore);
    return orig::SDL_CreateSemaphore(initial_value);
}

void SDL_DestroySemaphore(SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_DestroySemaphore);
    return orig::SDL_DestroySemaphore(sem);
}

int SDL_SemWait(SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_SemWait);
    return orig::SDL_SemWait(sem);
}

int SDL_SemTryWait(SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_SemTryWait);
    return orig::SDL_SemTryWait(sem);
}

int SDL_SemWaitTimeout(SDL_sem * sem, Uint32 ms)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_SemWaitTimeout);
    return orig::SDL_SemWaitTimeout(sem, ms);
}

int SDL_SemPost(SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_SemPost);
    return orig::SDL_SemPost(sem);
}

Uint32 SDL_SemValue(SDL_sem * sem)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_SemValue);
    return orig::SDL_SemValue(sem);
}

SDL_cond *SDL_CreateCond(void)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CreateCond);
    return orig::SDL_CreateCond();
}

void SDL_DestroyCond(SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_DestroyCond);
    return orig::SDL_DestroyCond(cond);
}

int SDL_CondSignal(SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CondSignal);
    return orig::SDL_CondSignal(cond);
}

int SDL_CondBroadcast(SDL_cond * cond)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CondBroadcast);
    return orig::SDL_CondBroadcast(cond);
}

int SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CondWait);
    return orig::SDL_CondWait(cond, mutex);
}

int SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    LOGTRACE(LCF_WAIT | LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_CondWaitTimeout);
    return orig::SDL_CondWaitTimeout(cond, mutex, ms);
}


}
