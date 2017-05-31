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

#include "threads.h"
#include "logging.h"
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <memory>
#include "checkpoint/ThreadInfo.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadSync.h"

/* Original function pointers */
namespace orig {
    static void* (*SDL_CreateThread)(int(*fn)(void*),
            const char*   name,
            void*         data);
    static void (*SDL_WaitThread)(void* thread, int *status);

    static int (*pthread_create) (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) = nullptr;
    static void __attribute__((__noreturn__)) (*pthread_exit) (void *retval) = nullptr;
    static int (*pthread_join) (unsigned long int thread, void **thread_return) = nullptr;
    static int (*pthread_detach) (unsigned long int thread) = nullptr;
    static int (*pthread_getname_np)(unsigned long int thread, char *name, size_t len) = nullptr;
    static int (*pthread_tryjoin_np)(unsigned long int thread, void **retval) = nullptr;
    static int (*pthread_timedjoin_np)(unsigned long int thread, void **retval, const struct timespec *abstime) = nullptr;
    static pthread_t (*pthread_self)(void) = nullptr;
    // static int (*pthread_cond_wait)(pthread_cond_t *cond, pthread_mutex_t *mutex) = nullptr;
    // static int (*pthread_cond_timedwait)(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) = nullptr;
    // static int (*pthread_cond_signal)(pthread_cond_t *cond) = nullptr;
    // static int (*pthread_cond_broadcast)(pthread_cond_t *cond) = nullptr;
    static int (*pthread_setcancelstate)(int state, int *oldstate) = nullptr;
    static int (*pthread_setcanceltype)(int type, int *oldtype) = nullptr;
    static int (*pthread_cancel)(pthread_t th) = nullptr;
    static void (*pthread_testcancel)(void) = nullptr;
}

/* We keep the identifier of the main thread */
pthread_t mainThread = 0;

/* Get the current thread id */
pthread_t getThreadId(void)
{
    LINK_NAMESPACE(pthread_self, "pthread");
    if (orig::pthread_self != nullptr)
        return orig::pthread_self();

    /* We couldn't link to pthread, meaning threading should be off.
     * We must return a value so that isMainThread() returns true.
     */
    return mainThread;
}

/* Indicate that we are running on the main thread */
void setMainThread(void)
{
    if (mainThread != 0)
        /* Main thread was already set */
        return;
    mainThread = getThreadId();
}

/*
 * We will often want to know if we are running on the main thread
 * Because only it can advance the deterministic timer, and other stuff
 */
int isMainThread(void)
{
    /* If threading is off, this will return true */
    return (getThreadId() == mainThread);
}

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

#if 0
    // NOTE: turn this to '#if 1' to impose de-sync on every single thread
    debuglog(LCF_THREAD, "WAITING for 2 sec");
    sleep(1);
#endif
    /* Wait for main thread to be ready to sleep */
    while(!thread->go)
        ;

    ThreadSync::decrementUninitializedThreadCount();
    void *ret = thread->start(thread->arg);
    debuglog(LCF_THREAD, "WE ARE DONE");
    // ThreadManager::resume(tid);
    ThreadManager::threadExit();
    // ThreadManager::end(thread->tid);
    return ret;
}


/* Override */ int pthread_create (pthread_t * tid_p, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
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

    // ThreadManager::start(*tid_p, __builtin_return_address(0), (void*)start_routine);

    LINK_NAMESPACE(pthread_getname_np, "pthread");
    char name[16];
    name[0] = '\0';
    orig::pthread_getname_np(*tid_p, name, 16);
    std::string thstr = stringify(*tid_p);
    if (name[0])
        debuglog(LCF_THREAD, "Thread ", thstr, " was created (", name, ").");
    else
        debuglog(LCF_THREAD, "Thread ", thstr, " was created.");
    debuglog(LCF_THREAD, "  - Entry point: ", (void*)start_routine, " .");

    /* Say to thread created that it can go! */
    thread->go = true;

    //Check and suspend main thread if needed
    // ThreadManager::suspend(*tid_p);

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
    // pthread_t tid = getThreadId();
    // ThreadManager::resume(tid);
    // ThreadManager::end(tid);
    orig::pthread_exit(retval);
}

/* Override */ int pthread_join (pthread_t thread, void **thread_return)
{
    LINK_NAMESPACE(pthread_join, "pthread");
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Joining thread ", thstr);
    int retVal = orig::pthread_join(thread, thread_return);
    ThreadManager::threadDetach(thread);
    // ThreadManager::get().end(thread);
    return retVal;
}

/* Override */ int pthread_detach (pthread_t thread) throw()
{
    LINK_NAMESPACE(pthread_detach, "pthread");
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Detaching thread ", thstr);
    // ThreadManager::resume(thread);
    int ret = orig::pthread_detach(thread);
    ThreadManager::threadDetach(thread);
    return ret;
}

/* Override */ int pthread_tryjoin_np(pthread_t thread, void **retval) throw()
{
    LINK_NAMESPACE(pthread_tryjoin_np, "pthread");
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr);
    int ret = orig::pthread_tryjoin_np(thread, retval);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == EBUSY) {
        debuglog(LCF_THREAD, "Thread ", thstr, " has not yet terminated.");
    }
    return ret;
}

/* Override */ int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime)
{
    LINK_NAMESPACE(pthread_timedjoin_np, "pthread");
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr, " in ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000," ms.");
    int ret = orig::pthread_timedjoin_np(thread, retval, abstime);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == ETIMEDOUT) {
        debuglog(LCF_THREAD, "Call timed out before thread ", thstr, " terminated.");
    }
    return ret;
}

/* Override */ int pthread_setcancelstate (int state, int *oldstate)
{
    LINK_NAMESPACE(pthread_setcancelstate, "pthread");
    DEBUGLOGCALL(LCF_THREAD);
    return orig::pthread_setcancelstate(state, oldstate);
}

/* Override */ int pthread_setcanceltype (int type, int *oldtype)
{
    LINK_NAMESPACE(pthread_setcanceltype, "pthread");
    DEBUGLOGCALL(LCF_THREAD);
    return orig::pthread_setcanceltype(type, oldtype);
}

/* Override */ int pthread_cancel (pthread_t th)
{
    LINK_NAMESPACE(pthread_cancel, "pthread");
    std::string thstr = stringify(th);
    debuglog(LCF_THREAD, "Cancel thread ", thstr);
    // ThreadManager::resume(th);
    // ThreadManager::end(th);
    return orig::pthread_cancel(th);
}

/* Override */ void pthread_testcancel (void)
{
    LINK_NAMESPACE(pthread_testcancel, "pthread");
    DEBUGLOGCALL(LCF_THREAD);
    return orig::pthread_testcancel();
}

void link_sdlthreads(void)
{
    LINK_NAMESPACE_SDLX(SDL_CreateThread);
    LINK_NAMESPACE_SDLX(SDL_WaitThread);
}
