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

#include "threadwrappers.h"
#include "logging.h"
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <memory>
#include <exception>
#include "checkpoint/ThreadInfo.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadSync.h"
#include "DeterministicTimer.h"
#include "backtrace.h"

namespace libtas {

DEFINE_ORIG_POINTER(SDL_CreateThread);
DEFINE_ORIG_POINTER(SDL_WaitThread);

DEFINE_ORIG_POINTER(pthread_create);
DEFINE_ORIG_POINTER(pthread_exit);
DEFINE_ORIG_POINTER(pthread_join);
DEFINE_ORIG_POINTER(pthread_detach);
DEFINE_ORIG_POINTER(pthread_tryjoin_np);
DEFINE_ORIG_POINTER(pthread_timedjoin_np);
DEFINE_ORIG_POINTER(pthread_cond_wait);
DEFINE_ORIG_POINTER(pthread_cond_timedwait);
DEFINE_ORIG_POINTER(pthread_cond_signal);
DEFINE_ORIG_POINTER(pthread_cond_broadcast);
DEFINE_ORIG_POINTER(pthread_setcancelstate);
DEFINE_ORIG_POINTER(pthread_setcanceltype);
DEFINE_ORIG_POINTER(pthread_cancel);
DEFINE_ORIG_POINTER(pthread_testcancel);
DEFINE_ORIG_POINTER(sem_timedwait);
DEFINE_ORIG_POINTER(sem_trywait);

/* We create a specific exception for thread exit calls */
class ThreadExitException: public std::exception {};

/* Override */ SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    debuglog(LCF_THREAD, "SDL Thread ", name, " was created.");
    return orig::SDL_CreateThread(fn, name, data);
}

/* Override */ void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    DEBUGLOGCALL(LCF_THREAD);
    orig::SDL_WaitThread(thread, status);
}

static void *pthread_start(void *arg)
{
    ThreadInfo *thread = static_cast<ThreadInfo*>(arg);

    std::unique_lock<std::mutex> lock(thread->mutex);

    /* Check if game is quitting */
    while (!thread->quit) {

        /* Check if there is a function to execute */
        if (thread->state == ThreadInfo::ST_RUNNING) {
            ThreadManager::update(thread);
            debuglog(LCF_THREAD, "Beginning of thread code ", thread->routine_id);

            ThreadSync::decrementUninitializedThreadCount();

            /* We need to handle the case where the thread calls pthread_exit to
             * terminate. Because we recycle thread routines, we must continue
             * the execution past the routine, so we are using the exception
             * feature for that.
             */
            void *ret;
            try {
                /* Execute the function */
                ret = thread->start(thread->arg);
            }
            catch (const ThreadExitException& e) {}

            debuglog(LCF_THREAD, "End of thread code");
            ThreadManager::threadExit(ret);

            /* Thread is now in zombie state until it is detached */
            while (thread->state == ThreadInfo::ST_ZOMBIE) {
                struct timespec mssleep = {0, 1000*1000};
                NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            }
        }
        else {
             thread->cv.wait(lock);
        }
    }

    return nullptr;
}


/* Override */ int pthread_create (pthread_t * tid_p, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
    debuglog(LCF_THREAD, "Thread is created with routine ", (void*)start_routine);
    LINK_NAMESPACE(pthread_create, "pthread");

    ThreadSync::wrapperExecutionLockLock();
    ThreadSync::incrementUninitializedThreadCount();

    /* Creating a new or recycled thread, and filling some parameters.
     * The rest (like thread->tid) will be filled by the child thread.
     */
    ThreadInfo* thread = ThreadManager::getNewThread();
    bool isRecycled = ThreadManager::initThread(thread, start_routine, arg, __builtin_return_address(0));

    int ret = 0;
    if (isRecycled) {
        debuglog(LCF_THREAD, "Recycling thread ", thread->tid);
        *tid_p = thread->pthread_id;
        /* Notify the thread that it has a function to execute */
        thread->cv.notify_all();
    }
    else {
        /* Call our wrapper function */
        ret = orig::pthread_create(tid_p, attr, pthread_start, thread);

        if (ret != 0) {
            /* Thread creation failed */
            ThreadSync::decrementUninitializedThreadCount();
            ThreadManager::threadIsDead(thread);
        }
    }

    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ void pthread_exit (void *retval)
{
    LINK_NAMESPACE(pthread_exit, "pthread");
    debuglog(LCF_THREAD, "Thread has exited.");

    /* We need to jump to code after the end of the original thread routine */
    throw ThreadExitException();

    // ThreadInfo* thread = ThreadManager::getThread(ThreadManager::getThreadId());
    // longjmp(thread->env, 1);
}

/* Override */ int pthread_join (pthread_t pthread_id, void **thread_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pthread_join, "pthread");
        return orig::pthread_join(pthread_id, thread_return);
    }

    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD, "Joining thread ", ThreadManager::getThreadTid(pthread_id));

    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        ThreadSync::wrapperExecutionLockUnlock();
        return ESRCH;
    }

    if (thread->detached) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    /* Wait for the thread to become zombie */
    while (thread->state != ThreadInfo::ST_ZOMBIE) {
        struct timespec mssleep = {0, 1000*1000};
        NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
    }

    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return 0;
}

/* Override */ int pthread_detach (pthread_t pthread_id) throw()
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pthread_detach, "pthread");
        return orig::pthread_detach(pthread_id);
    }

    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD, "Detaching thread ", ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        ThreadSync::wrapperExecutionLockUnlock();
        return ESRCH;
    }

    if (thread->detached) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return 0;
}

/* Override */ int pthread_tryjoin_np(pthread_t pthread_id, void **retval) throw()
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pthread_tryjoin_np, "pthread");
        return orig::pthread_tryjoin_np(pthread_id, retval);
    }

    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD, "Try to join thread ", ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        ThreadSync::wrapperExecutionLockUnlock();
        return ESRCH;
    }

    if (thread->detached) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    if (thread->state == ThreadInfo::ST_ZOMBIE) {
        debuglog(LCF_THREAD, "Joining thread successfully.");
        if (retval) {
            *retval = thread->retval;
        }
        ThreadManager::threadDetach(pthread_id);
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }

    debuglog(LCF_THREAD, "Thread has not yet terminated.");
    ThreadSync::wrapperExecutionLockUnlock();
    return EBUSY;
}

/* Override */ int pthread_timedjoin_np(pthread_t pthread_id, void **retval, const struct timespec *abstime)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pthread_timedjoin_np, "pthread");
        return orig::pthread_timedjoin_np(pthread_id, retval, abstime);
    }

    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD | LCF_TODO, "Try to join thread in ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000," ms.");

    if (abstime->tv_sec < 0 || abstime->tv_nsec >= 1000000000) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        ThreadSync::wrapperExecutionLockUnlock();
        return ESRCH;
    }

    if (thread->detached) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    /* For now I'm lazy, so we just wait the amount of time and check joining */
    NATIVECALL(nanosleep(abstime, NULL));

    if (thread->state == ThreadInfo::ST_ZOMBIE) {
        debuglog(LCF_THREAD, "Joining thread successfully.");
        if (retval) {
            *retval = thread->retval;
        }
        ThreadManager::threadDetach(pthread_id);
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }

    debuglog(LCF_THREAD, "Call timed out before thread terminated.");
    ThreadSync::wrapperExecutionLockUnlock();
    return ETIMEDOUT;
}

/* Override */ int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    LINK_NAMESPACE_VERSION(pthread_cond_wait, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO | (ThreadManager::isMainThread()?0:LCF_FREQUENT), __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex));
    return orig::pthread_cond_wait(cond, mutex);
}

/* Override */ int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    LINK_NAMESPACE_VERSION(pthread_cond_timedwait, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO | (ThreadManager::isMainThread()?0:LCF_FREQUENT), __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex));
    return orig::pthread_cond_timedwait(cond, mutex, abstime);
}

/* Override */ int pthread_cond_signal(pthread_cond_t *cond) throw()
{
    LINK_NAMESPACE_VERSION(pthread_cond_signal, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO | (ThreadManager::isMainThread()?0:LCF_FREQUENT), __func__, " call with cond ", static_cast<void*>(cond));
    return orig::pthread_cond_signal(cond);
}

/* Override */ int pthread_cond_broadcast(pthread_cond_t *cond) throw()
{
    LINK_NAMESPACE_VERSION(pthread_cond_broadcast, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO | (ThreadManager::isMainThread()?0:LCF_FREQUENT), __func__, " call with cond ", static_cast<void*>(cond));
//    if (ThreadManager::isMainThread()) printBacktrace();
    return orig::pthread_cond_broadcast(cond);
}

/* Override */ int pthread_setcancelstate (int state, int *oldstate)
{
    LINK_NAMESPACE(pthread_setcancelstate, "pthread");
    DEBUGLOGCALL(LCF_THREAD | LCF_TODO);
    return orig::pthread_setcancelstate(state, oldstate);
}

/* Override */ int pthread_setcanceltype (int type, int *oldtype)
{
    LINK_NAMESPACE(pthread_setcanceltype, "pthread");
    DEBUGLOGCALL(LCF_THREAD | LCF_TODO);
    return orig::pthread_setcanceltype(type, oldtype);
}

/* Override */ int pthread_cancel (pthread_t pthread_id)
{
    LINK_NAMESPACE(pthread_cancel, "pthread");
    debuglog(LCF_THREAD | LCF_TODO, "Cancel thread ", ThreadManager::getThreadTid(pthread_id));
    return orig::pthread_cancel(pthread_id);
}

/* Override */ void pthread_testcancel (void)
{
    LINK_NAMESPACE(pthread_testcancel, "pthread");
    DEBUGLOGCALL(LCF_THREAD | LCF_TODO);
    return orig::pthread_testcancel();
}

int sem_timedwait (sem_t * sem, const struct timespec *abstime)
{
    LINK_NAMESPACE(sem_timedwait, "pthread");
    DEBUGLOGCALL(LCF_THREAD | LCF_TODO);
    return orig::sem_timedwait(sem, abstime);
}

int sem_trywait (sem_t *sem) throw()
{
    LINK_NAMESPACE(sem_trywait, "pthread");
    DEBUGLOGCALL(LCF_THREAD | LCF_TODO);
    return orig::sem_trywait(sem);
}

void link_sdlthreads(void)
{
    LINK_NAMESPACE_SDLX(SDL_CreateThread);
    LINK_NAMESPACE_SDLX(SDL_WaitThread);
}

}
