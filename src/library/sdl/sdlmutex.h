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

#ifndef LIBTAS_SDLMUTEX_H_INCL
#define LIBTAS_SDLMUTEX_H_INCL

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

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
 * Wait until a semaphore has a positive value and then decrements it.
 *
 * This function suspends the calling thread until the semaphore pointed to by
 * `sem` has a positive value, and then atomically decrement the semaphore
 * value.
 *
 * This function is the equivalent of calling SDL_WaitSemaphoreTimeout() with
 * a time length of -1.
 *
 * \param sem the semaphore wait on.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SignalSemaphore
 * \sa SDL_TryWaitSemaphore
 * \sa SDL_WaitSemaphoreTimeout
 */
OVERRIDE void SDL_WaitSemaphore(sdl3::SDL_Semaphore *sem);

/**
 *  Non-blocking variant of SDL_SemWait().
 *
 *  \return 0 if the wait succeeds, ::SDL_MUTEX_TIMEDOUT if the wait would
 *          block, and -1 on error.
 */
OVERRIDE int SDL_SemTryWait(SDL_sem * sem);

/**
 * See if a semaphore has a positive value and decrement it if it does.
 *
 * This function checks to see if the semaphore pointed to by `sem` has a
 * positive value and atomically decrements the semaphore value if it does. If
 * the semaphore doesn't have a positive value, the function immediately
 * returns false.
 *
 * \param sem the semaphore to wait on.
 * \returns true if the wait succeeds, false if the wait would block.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SignalSemaphore
 * \sa SDL_WaitSemaphore
 * \sa SDL_WaitSemaphoreTimeout
 */
OVERRIDE bool SDL_TryWaitSemaphore(sdl3::SDL_Semaphore *sem);

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
 * Wait until a semaphore has a positive value and then decrements it.
 *
 * This function suspends the calling thread until either the semaphore
 * pointed to by `sem` has a positive value or the specified time has elapsed.
 * If the call is successful it will atomically decrement the semaphore value.
 *
 * \param sem the semaphore to wait on.
 * \param timeoutMS the length of the timeout, in milliseconds, or -1 to wait
 *                  indefinitely.
 * \returns true if the wait succeeds or false if the wait times out.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SignalSemaphore
 * \sa SDL_TryWaitSemaphore
 * \sa SDL_WaitSemaphore
 */
OVERRIDE bool SDL_WaitSemaphoreTimeout(sdl3::SDL_Semaphore *sem, Sint32 timeoutMS);

/**
 *  Atomically increases the semaphore's count (not blocking).
 *
 *  \return 0, or -1 on error.
 */
OVERRIDE int SDL_SemPost(SDL_sem * sem);

/**
 * Atomically increment a semaphore's value and wake waiting threads.
 *
 * \param sem the semaphore to increment.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_TryWaitSemaphore
 * \sa SDL_WaitSemaphore
 * \sa SDL_WaitSemaphoreTimeout
 */
OVERRIDE void SDL_SignalSemaphore(sdl3::SDL_Semaphore *sem);

/**
 *  Returns the current count of the semaphore.
 */
OVERRIDE Uint32 SDL_SemValue(SDL_sem * sem);

/**
 * Get the current value of a semaphore.
 *
 * \param sem the semaphore to query.
 * \returns the current value of the semaphore.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE Uint32 SDL_GetSemaphoreValue(sdl3::SDL_Semaphore *sem);

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
 * Create a condition variable.
 *
 * \returns a new condition variable or NULL on failure; call SDL_GetError()
 *          for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BroadcastCondition
 * \sa SDL_SignalCondition
 * \sa SDL_WaitCondition
 * \sa SDL_WaitConditionTimeout
 * \sa SDL_DestroyCondition
 */
OVERRIDE sdl3::SDL_Condition * SDL_CreateCondition(void);

/**
 *  Destroy a condition variable.
 */
OVERRIDE void SDL_DestroyCond(SDL_cond * cond);

/**
 * Destroy a condition variable.
 *
 * \param cond the condition variable to destroy.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreateCondition
 */
OVERRIDE void SDL_DestroyCondition(sdl3::SDL_Condition *cond);

/**
 *  Restart one of the threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
OVERRIDE int SDL_CondSignal(SDL_cond * cond);

/**
 * Restart one of the threads that are waiting on the condition variable.
 *
 * \param cond the condition variable to signal.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BroadcastCondition
 * \sa SDL_WaitCondition
 * \sa SDL_WaitConditionTimeout
 */
OVERRIDE void SDL_SignalCondition(sdl3::SDL_Condition *cond);

/**
 *  Restart all threads that are waiting on the condition variable.
 *
 *  \return 0 or -1 on error.
 */
OVERRIDE int SDL_CondBroadcast(SDL_cond * cond);

/**
 * Restart all threads that are waiting on the condition variable.
 *
 * \param cond the condition variable to signal.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SignalCondition
 * \sa SDL_WaitCondition
 * \sa SDL_WaitConditionTimeout
 */
OVERRIDE void SDL_BroadcastCondition(sdl3::SDL_Condition *cond);

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
 * Wait until a condition variable is signaled.
 *
 * This function unlocks the specified `mutex` and waits for another thread to
 * call SDL_SignalCondition() or SDL_BroadcastCondition() on the condition
 * variable `cond`. Once the condition variable is signaled, the mutex is
 * re-locked and the function returns.
 *
 * The mutex must be locked before calling this function. Locking the mutex
 * recursively (more than once) is not supported and leads to undefined
 * behavior.
 *
 * This function is the equivalent of calling SDL_WaitConditionTimeout() with
 * a time length of -1.
 *
 * \param cond the condition variable to wait on.
 * \param mutex the mutex used to coordinate thread access.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BroadcastCondition
 * \sa SDL_SignalCondition
 * \sa SDL_WaitConditionTimeout
 */
OVERRIDE void SDL_WaitCondition(sdl3::SDL_Condition *cond, sdl3::SDL_Mutex *mutex);

/**
 *  Waits for at most \c ms milliseconds, and returns 0 if the condition
 *  variable is signaled, ::SDL_MUTEX_TIMEDOUT if the condition is not
 *  signaled in the allotted time, and -1 on error.
 *
 *  \warning On some platforms this function is implemented by looping with a
 *           delay of 1 ms, and so should be avoided if possible.
 */
OVERRIDE int SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms);

/**
 * Wait until a condition variable is signaled or a certain time has passed.
 *
 * This function unlocks the specified `mutex` and waits for another thread to
 * call SDL_SignalCondition() or SDL_BroadcastCondition() on the condition
 * variable `cond`, or for the specified time to elapse. Once the condition
 * variable is signaled or the time elapsed, the mutex is re-locked and the
 * function returns.
 *
 * The mutex must be locked before calling this function. Locking the mutex
 * recursively (more than once) is not supported and leads to undefined
 * behavior.
 *
 * \param cond the condition variable to wait on.
 * \param mutex the mutex used to coordinate thread access.
 * \param timeoutMS the maximum time to wait, in milliseconds, or -1 to wait
 *                  indefinitely.
 * \returns true if the condition variable is signaled, false if the condition
 *          is not signaled in the allotted time.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BroadcastCondition
 * \sa SDL_SignalCondition
 * \sa SDL_WaitCondition
 */
OVERRIDE bool SDL_WaitConditionTimeout(sdl3::SDL_Condition *cond, sdl3::SDL_Mutex *mutex, Sint32 timeoutMS);

}

#endif
