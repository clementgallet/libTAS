/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLMUTEX_H_INCL
#define LIBTAS_SDLMUTEX_H_INCL

#include <SDL2/SDL.h>
#include "global.h"

namespace libtas {

/**
 *  Create a mutex, initialized unlocked.
 */
OVERRIDE SDL_mutex* SDL_CreateMutex(void);

/**
 *  Lock the mutex.
 *
 *  \return 0, or -1 on error.
 */
OVERRIDE int SDL_LockMutex(SDL_mutex * mutex);

/**
 *  Try to lock the mutex
 *
 *  \return 0, SDL_MUTEX_TIMEDOUT, or -1 on error
 */
OVERRIDE int SDL_TryLockMutex(SDL_mutex * mutex);

/**
 *  Unlock the mutex.
 *
 *  \return 0, or -1 on error.
 *
 *  \warning It is an error to unlock a mutex that has not been locked by
 *           the current thread, and doing so results in undefined behavior.
 */
OVERRIDE int SDL_UnlockMutex(SDL_mutex * mutex);

/**
 *  Destroy a mutex.
 */
OVERRIDE void SDL_DestroyMutex(SDL_mutex * mutex);

/**
 *  Create a semaphore, initialized with value, returns NULL on failure.
 */
OVERRIDE SDL_sem *SDL_CreateSemaphore(Uint32 initial_value);

/**
 *  Destroy a semaphore.
 */
OVERRIDE void SDL_DestroySemaphore(SDL_sem * sem);

/**
 *  This function suspends the calling thread until the semaphore pointed
 *  to by \c sem has a positive count. It then atomically decreases the
 *  semaphore count.
 */
OVERRIDE int SDL_SemWait(SDL_sem * sem);

/**
 *  Non-blocking variant of SDL_SemWait().
 *
 *  \return 0 if the wait succeeds, ::SDL_MUTEX_TIMEDOUT if the wait would
 *          block, and -1 on error.
 */
OVERRIDE int SDL_SemTryWait(SDL_sem * sem);

/**
 *  Variant of SDL_SemWait() with a timeout in milliseconds.
 *
 *  \return 0 if the wait succeeds, ::SDL_MUTEX_TIMEDOUT if the wait does not
 *          succeed in the allotted time, and -1 on error.
 *
 *  \warning On some platforms this function is implemented by looping with a
 *           delay of 1 ms, and so should be avoided if possible.
 */
OVERRIDE int SDL_SemWaitTimeout(SDL_sem * sem, Uint32 ms);

/**
 *  Atomically increases the semaphore's count (not blocking).
 *
 *  \return 0, or -1 on error.
 */
OVERRIDE int SDL_SemPost(SDL_sem * sem);

/**
 *  Returns the current count of the semaphore.
 */
OVERRIDE Uint32 SDL_SemValue(SDL_sem * sem);

/**
 *  Create a condition variable.
 *
 *  Typical use of condition variables:
 *
 *  Thread A:
 *    SDL_LockMutex(lock);
 *    while ( ! condition ) {
 *        SDL_CondWait(cond, lock);
 *    }
 *    SDL_UnlockMutex(lock);
 *
 *  Thread B:
 *    SDL_LockMutex(lock);
 *    ...
 *    condition = true;
 *    ...
 *    SDL_CondSignal(cond);
 *    SDL_UnlockMutex(lock);
 *
 *  There is some discussion whether to signal the condition variable
 *  with the mutex locked or not.  There is some potential performance
 *  benefit to unlocking first on some platforms, but there are some
 *  potential race conditions depending on how your code is structured.
 *
 *  In general it's safer to signal the condition variable while the
 *  mutex is locked.
 */
OVERRIDE SDL_cond *SDL_CreateCond(void);

/**
 *  Destroy a condition variable.
 */
OVERRIDE void SDL_DestroyCond(SDL_cond * cond);

/**
 *  Restart one of the threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
OVERRIDE int SDL_CondSignal(SDL_cond * cond);

/**
 *  Restart all threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
OVERRIDE int SDL_CondBroadcast(SDL_cond * cond);

/**
 *  Wait on the condition variable, unlocking the provided mutex.
 *
 *  \warning The mutex must be locked before entering this function!
 *
 *  The mutex is re-locked once the condition variable is signaled.
 *
 *  \return 0 when it is signaled, or -1 on error.
 */
OVERRIDE int SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex);

/**
 *  Waits for at most \c ms milliseconds, and returns 0 if the condition
 *  variable is signaled, ::SDL_MUTEX_TIMEDOUT if the condition is not
 *  signaled in the allotted time, and -1 on error.
 *
 *  \warning On some platforms this function is implemented by looping with a
 *           delay of 1 ms, and so should be avoided if possible.
 */
OVERRIDE int SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms);

}

#endif
