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
#include "checkpoint/ThreadInfo.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadSync.h"

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
    ThreadManager::update(thread);

    debuglog(LCF_THREAD, "Beginning of thread code");

#if 0
    // NOTE: turn this to '#if 1' to impose de-sync on every single thread
    debuglog(LCF_THREAD, "WAITING for 2 sec");
    sleep(1);
#endif

    ThreadSync::decrementUninitializedThreadCount();
    void *ret = thread->start(thread->arg);
    debuglog(LCF_THREAD, "End of thread code");
    ThreadManager::threadExit();
    return ret;
}


/* Override */ int pthread_create (pthread_t * tid_p, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
    debuglog(LCF_THREAD, "Thread is created with routine ", (void*)start_routine);
    LINK_NAMESPACE(pthread_create, "pthread");

    ThreadSync::wrapperExecutionLockLock();
    ThreadSync::incrementUninitializedThreadCount();

    /* Creating a new thread and filling some parameters.
     * The rest (like thread->tid) will be filled by the child thread.
     */
    ThreadInfo* thread = ThreadManager::getNewThread();
    ThreadManager::initThread(thread, start_routine, arg, __builtin_return_address(0));

    /* Call our wrapper function */
    int ret = orig::pthread_create(tid_p, attr, pthread_start, thread);

    if (ret != 0) {
        /* Thread creation failed */
        ThreadSync::decrementUninitializedThreadCount();
        ThreadManager::threadIsDead(thread);
        return ret;
    }

    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ void pthread_exit (void *retval)
{
    LINK_NAMESPACE(pthread_exit, "pthread");
    debuglog(LCF_THREAD, "Thread has exited.");
    ThreadSync::wrapperExecutionLockLock();
    ThreadManager::threadExit();
    ThreadSync::wrapperExecutionLockUnlock();
    orig::pthread_exit(retval);
}

/* Override */ int pthread_join (pthread_t pthread_id, void **thread_return)
{
    LINK_NAMESPACE(pthread_join, "pthread");

    if (GlobalState::isNative())
        return orig::pthread_join(pthread_id, thread_return);

    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD, "Joining thread ", ThreadManager::getThreadTid(pthread_id));

    /* Because we detach zombie threads before saving or loading a state,
     * this thread might not be there anymore. So we check in our thread list
     * and don't detach/join if in fake zombie state. If so, we can return
     * the value that we stored when we joined the thread.
     */
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);
    if (thread && thread->state == ThreadInfo::ST_FAKEZOMBIE) {
        /* Thread is now officially joined, we can remove the thread from
        * our list.
        */
        *thread_return = thread->retval;
        ThreadManager::threadIsDead(thread);
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }

    int ret = orig::pthread_join(pthread_id, thread_return);
    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ int pthread_detach (pthread_t pthread_id) throw()
{
    LINK_NAMESPACE(pthread_detach, "pthread");

    if (GlobalState::isNative())
        return orig::pthread_detach(pthread_id);

    /* Same comment as above */
    ThreadSync::wrapperExecutionLockLock();
    debuglog(LCF_THREAD, "Detaching thread ", ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);
    if (thread && thread->state == ThreadInfo::ST_FAKEZOMBIE) {
        /* Thread is now officially detached, we can remove the thread from
         * our list.
         */
        ThreadManager::threadIsDead(thread);
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }

    int ret = orig::pthread_detach(pthread_id);
    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ int pthread_tryjoin_np(pthread_t pthread_id, void **retval) throw()
{
    LINK_NAMESPACE(pthread_tryjoin_np, "pthread");
    pid_t tid = ThreadManager::getThreadTid(pthread_id);
    debuglog(LCF_THREAD | LCF_TODO, "Try to join thread ", tid);
    int ret = orig::pthread_tryjoin_np(pthread_id, retval);
    if (ret == 0) {
        debuglog(LCF_THREAD | LCF_TODO, "Joining thread ", tid, " successfully.");
    }
    if (ret == EBUSY) {
        debuglog(LCF_THREAD | LCF_TODO, "Thread ", tid, " has not yet terminated.");
    }
    return ret;
}

/* Override */ int pthread_timedjoin_np(pthread_t pthread_id, void **retval, const struct timespec *abstime)
{
    LINK_NAMESPACE(pthread_timedjoin_np, "pthread");
    pid_t tid = ThreadManager::getThreadTid(pthread_id);
    debuglog(LCF_THREAD | LCF_TODO, "Try to join thread ", tid, " in ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000," ms.");
    int ret = orig::pthread_timedjoin_np(pthread_id, retval, abstime);
    if (ret == 0) {
        debuglog(LCF_THREAD | LCF_TODO, "Joining thread ", tid, " successfully.");
    }
    if (ret == ETIMEDOUT) {
        debuglog(LCF_THREAD | LCF_TODO, "Call timed out before thread ", tid, " terminated.");
    }
    return ret;
}

/* Override */ int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    LINK_NAMESPACE_VERSION(pthread_cond_wait, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex));
    return orig::pthread_cond_wait(cond, mutex);
}

/* Override */ int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    LINK_NAMESPACE_VERSION(pthread_cond_timedwait, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex));
    return orig::pthread_cond_timedwait(cond, mutex, abstime);
}

/* Override */ int pthread_cond_signal(pthread_cond_t *cond) throw()
{
    LINK_NAMESPACE_VERSION(pthread_cond_signal, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond));
    return orig::pthread_cond_signal(cond);
}

/* Override */ int pthread_cond_broadcast(pthread_cond_t *cond) throw()
{
    LINK_NAMESPACE_VERSION(pthread_cond_broadcast, "pthread", "GLIBC_2.3.2");
    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond));
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

void link_sdlthreads(void)
{
    LINK_NAMESPACE_SDLX(SDL_CreateThread);
    LINK_NAMESPACE_SDLX(SDL_WaitThread);
}

}
