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

    /* Override */ SDL_mutex* SDL_CreateMutex(void)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_CreateMutex, ());
}

int SDL_LockMutex(SDL_mutex * mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_LockMutex, (mutex));
}

int SDL_TryLockMutex(SDL_mutex * mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_TryLockMutex, (mutex));
}

int SDL_UnlockMutex(SDL_mutex * mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_UnlockMutex, (mutex));
}

void SDL_DestroyMutex(SDL_mutex * mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_DestroyMutex, (mutex));
}

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_CreateSemaphore, (initial_value));
}

void SDL_DestroySemaphore(SDL_sem * sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL23_CALL(SDL_DestroySemaphore, (sem));
}

int SDL_SemWait(SDL_sem * sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemWait, (sem));
}

void SDL_WaitSemaphore(sdl3::SDL_Semaphore *sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_WaitSemaphore, (sem));
}

int SDL_SemTryWait(SDL_sem * sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemTryWait, (sem));
}

bool SDL_TryWaitSemaphore(sdl3::SDL_Semaphore *sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL3_CALL(SDL_TryWaitSemaphore, (sem));
}

int SDL_SemWaitTimeout(SDL_sem * sem, Uint32 ms)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemWaitTimeout, (sem, ms));
}

bool SDL_WaitSemaphoreTimeout(sdl3::SDL_Semaphore *sem, Sint32 timeoutMS)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL3_CALL(SDL_WaitSemaphoreTimeout, (sem, timeoutMS));
}

int SDL_SemPost(SDL_sem * sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemPost, (sem));
}

void SDL_SignalSemaphore(sdl3::SDL_Semaphore *sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_SignalSemaphore, (sem));
}

Uint32 SDL_SemValue(SDL_sem * sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_SemValue, (sem));
}

Uint32 SDL_GetSemaphoreValue(sdl3::SDL_Semaphore *sem)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL3_CALL(SDL_GetSemaphoreValue, (sem));
}

SDL_cond *SDL_CreateCond(void)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CreateCond, ());
}

sdl3::SDL_Condition * SDL_CreateCondition(void)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL3_CALL(SDL_CreateCondition, ());
}

void SDL_DestroyCond(SDL_cond * cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_DestroyCond, (cond));
}

void SDL_DestroyCondition(sdl3::SDL_Condition *cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_DestroyCondition, (cond));
}

int SDL_CondSignal(SDL_cond * cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondSignal, (cond));
}

void SDL_SignalCondition(sdl3::SDL_Condition *cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_SignalCondition, (cond));
}

int SDL_CondBroadcast(SDL_cond * cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondBroadcast, (cond));
}

void SDL_BroadcastCondition(sdl3::SDL_Condition *cond)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_BroadcastCondition, (cond));
}

int SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondWait, (cond, mutex));
}

void SDL_WaitCondition(sdl3::SDL_Condition *cond, sdl3::SDL_Mutex *mutex)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    ORIG_SDL3_CALL(SDL_WaitCondition, (cond, mutex));
}

int SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL2_CALL(SDL_CondWaitTimeout, (cond, mutex, ms));
}

bool SDL_WaitConditionTimeout(sdl3::SDL_Condition *cond, sdl3::SDL_Mutex *mutex, Sint32 timeoutMS)
{
    LOGTRACE_SIMPLE(LCF_WAIT | LCF_SDL);
    return ORIG_SDL3_CALL(SDL_WaitConditionTimeout, (cond, mutex, timeoutMS));
}

}
