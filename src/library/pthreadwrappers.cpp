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

#include "pthreadwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadInfo.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadSync.h"
#include "DeterministicTimer.h"
#include "tlswrappers.h"
#include "backtrace.h"
#include "hook.h"

#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <memory>
#include <exception>

namespace libtas {

DEFINE_ORIG_POINTER(pthread_create)
DEFINE_ORIG_POINTER(pthread_exit)
DEFINE_ORIG_POINTER(pthread_join)
DEFINE_ORIG_POINTER(pthread_detach)
DEFINE_ORIG_POINTER(pthread_tryjoin_np)
DEFINE_ORIG_POINTER(pthread_timedjoin_np)
DEFINE_ORIG_POINTER(pthread_cond_init)
DEFINE_ORIG_POINTER(pthread_cond_wait)
DEFINE_ORIG_POINTER(pthread_cond_timedwait)
DEFINE_ORIG_POINTER(pthread_cond_signal)
DEFINE_ORIG_POINTER(pthread_cond_broadcast)
DEFINE_ORIG_POINTER(pthread_setcancelstate)
DEFINE_ORIG_POINTER(pthread_setcanceltype)
DEFINE_ORIG_POINTER(pthread_cancel)
DEFINE_ORIG_POINTER(pthread_testcancel)
DEFINE_ORIG_POINTER(sem_wait)
DEFINE_ORIG_POINTER(sem_timedwait)
DEFINE_ORIG_POINTER(sem_trywait)
DEFINE_ORIG_POINTER(pthread_attr_setstack)
DEFINE_ORIG_POINTER(pthread_condattr_setclock)
DEFINE_ORIG_POINTER(pthread_condattr_getclock)
DEFINE_ORIG_POINTER(pthread_setname_np)

/* We create a specific exception for thread exit calls */
class ThreadExitException {
    const char* what () const throw ()
    {
    	return "Thread exit";
    }
};

extern "C" {
  // Declare an extern reference to the internal, undocumented
  // function that glibc uses to set (or reset) thread-local storage
  // to its initial state.

  // This function (effectively) takes a pthread_t argument on
  // 32-bit and 64-bit x86 Linux, but for some other architectures
  // it does not.  If you search the glibc source code for TLS_TCB_AT_TP,
  // architectures that define it as 1 should work with this;
  // architectures that define it as 0 would require a different
  // code path.

  // Even though this function is internal and undocumented, I don't think
  // it's likely to change; it's part of the ABI between libpthread.so
  // and ld-linux.so.
  extern void _dl_allocate_tls_init(pthread_t pt);

  // Another internal, undocumented libc function, that calls C++
  // destructors on thread-local storage.
  extern void __call_tls_dtors();

  // Another internal, undocumented libc function, that cleans up
  // any libc internal state in thread-local storage.
  extern void __libc_thread_freeres();
}

static void *pthread_start(void *arg)
{
    ThreadInfo *thread = static_cast<ThreadInfo*>(arg);

    std::unique_lock<std::mutex> lock(thread->mutex);

    ThreadManager::initThreadFromChild(thread);

    do {
        /* Check if there is a function to execute */
        if (thread->state == ThreadInfo::ST_RUNNING) {
            ThreadManager::update(thread);
            ThreadSync::decrementUninitializedThreadCount();

            debuglog(LCF_THREAD, "Beginning of thread code ", thread->routine_id);

            /* We need to handle the case where the thread calls pthread_exit to
             * terminate. Because we recycle thread routines, we must continue
             * the execution past the routine, so we are using the exception
             * feature for that.
             */
            void *ret = nullptr;
            try {
                /* Execute the function */
                ret = thread->start(thread->arg);
            }
            catch (const ThreadExitException& e) {}

            debuglog(LCF_THREAD, "End of thread code");

            /* Because we recycle this thread, we must unset all TLS values
             * and call destructors ourselves.  First, we unset the values
             * from the older, pthread_key_create()-based implementation
             * of TLS.
             */
            clear_pthread_keys();
            /* Next we deal with the newer linker-based TLS
             * implementation accessed via thread_local in C11/C++11
             * and later.  For that, first we need to run any C++
             * destructors for values in thread-local storage, using
             * an internal libc function.
             */
            __call_tls_dtors();
            /* Next, we clean up any libc state in thread-local storage,
             * using an internal libc function.
             */
            __libc_thread_freeres();
            /* Finally, we reset all thread-local storage back to its
             * initial value, using a third internal libc function.
             * This is architecture-specific; it works on 32-bit and
             * 64-bit x86, but not on all Linux architectures.  See
             * above, where this function is declared.
             */
            _dl_allocate_tls_init(thread->pthread_id);
            /* This has just reset any libTAS thread-local storage
             * for this thread.  Most libTAS TLS is either transient
             * anyway, or irrelevant while the thread is waiting
             * to be recycled.  But one value is important:
             * we need to fix ThreadManager::current_thread .
             */
            ThreadManager::setCurrentThread(thread);

            ThreadManager::threadExit(ret);

            /* Thread is now in zombie state until it is detached */
            // while (thread->state == ThreadInfo::ST_ZOMBIE) {
            //     struct timespec mssleep = {0, 1000*1000};
            //     NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            // }
        }
        else {
             thread->cv.wait(lock);
        }
    } while (!thread->quit && shared_config.recycle_threads); /* Check if game is quitting */

    return nullptr;
}


/* Override */ int pthread_create (pthread_t * tid_p, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) __THROW
{
    LINK_NAMESPACE(pthread_create, "pthread");

    if (GlobalState::isNative())
        return orig::pthread_create(tid_p, attr, start_routine, arg);

    debuglog(LCF_THREAD, "Thread is created with routine ", (void*)start_routine);

    ThreadSync::wrapperExecutionLockLock();
    ThreadSync::incrementUninitializedThreadCount();

    /* Creating a new or recycled thread, and filling some parameters.
     * The rest (like thread->tid) will be filled by the child thread.
     */
    ThreadInfo* thread = ThreadManager::getNewThread();
    bool isRecycled = ThreadManager::initThreadFromParent(thread, start_routine, arg, __builtin_return_address(0));

    /* Threads can be created in detached state */
    int detachstate = PTHREAD_CREATE_JOINABLE; // default
    if (attr) {
        pthread_attr_getdetachstate(attr, &detachstate);
        debuglog(LCF_THREAD, "Detached state is ", detachstate);
        debuglog(LCF_THREAD, "Default state is ", PTHREAD_CREATE_JOINABLE);
    }
    thread->detached = (detachstate == PTHREAD_CREATE_DETACHED);

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

    if (GlobalState::isNative())
        return orig::pthread_exit(retval);

    debuglog(LCF_THREAD, "Thread has exited.");

    if (shared_config.recycle_threads) {
        /* We need to jump to code after the end of the original thread routine */
        throw ThreadExitException();

        // ThreadInfo* thread = ThreadManager::getThread(ThreadManager::getThreadId());
        // longjmp(thread->env, 1);
    }
    else {
        ThreadManager::threadExit(retval);
        orig::pthread_exit(retval);
    }
}

/* Override */ int pthread_join (pthread_t pthread_id, void **thread_return)
{
    LINK_NAMESPACE(pthread_join, "pthread");
    if (GlobalState::isNative()) {
        return orig::pthread_join(pthread_id, thread_return);
    }

    ThreadSync::waitForThreadsToFinishInitialization();

    debuglog(LCF_THREAD, "Joining thread id ", pthread_id, " tid " , ThreadManager::getThreadTid(pthread_id));

    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        return ESRCH;
    }

    if (thread->detached) {
        return EINVAL;
    }

    int ret = 0;
    if (shared_config.recycle_threads) {
        /* Wait for the thread to become zombie */
        while (thread->state != ThreadInfo::ST_ZOMBIE) {
            struct timespec mssleep = {0, 1000*1000};
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
        }
    }
    else {
        ret = orig::pthread_join(pthread_id, thread_return);
    }

    ThreadSync::wrapperExecutionLockLock();
    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ int pthread_detach (pthread_t pthread_id) __THROW
{
    LINK_NAMESPACE(pthread_detach, "pthread");
    if (GlobalState::isNative()) {
        return orig::pthread_detach(pthread_id);
    }

    ThreadSync::wrapperExecutionLockLock();
    ThreadSync::waitForThreadsToFinishInitialization();

    debuglog(LCF_THREAD, "Detaching thread id ", pthread_id, " tid ", ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        ThreadSync::wrapperExecutionLockUnlock();
        return ESRCH;
    }

    if (thread->detached) {
        ThreadSync::wrapperExecutionLockUnlock();
        return EINVAL;
    }

    int ret = 0;
    if (! shared_config.recycle_threads) {
        ret = orig::pthread_detach(pthread_id);
    }

    ThreadManager::threadDetach(pthread_id);
    ThreadSync::wrapperExecutionLockUnlock();
    return ret;
}

/* Override */ int pthread_tryjoin_np(pthread_t pthread_id, void **retval) __THROW
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pthread_tryjoin_np, "pthread");
        return orig::pthread_tryjoin_np(pthread_id, retval);
    }

    ThreadSync::wrapperExecutionLockLock();
    ThreadSync::waitForThreadsToFinishInitialization();

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

    int ret = 0;
    if (shared_config.recycle_threads) {
        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            if (retval) {
                *retval = thread->retval;
            }
            ThreadManager::threadDetach(pthread_id);
        }
        else {
            ret = EBUSY;
        }
    }
    else {
        ret = orig::pthread_tryjoin_np(pthread_id, retval);
        if (ret == 0) {
            ThreadManager::threadDetach(pthread_id);
        }
    }

    if (ret == 0)
        debuglog(LCF_THREAD, "Joining thread successfully.");
    else
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
    ThreadSync::waitForThreadsToFinishInitialization();

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

    int ret = 0;
    if (shared_config.recycle_threads) {
        /* For now I'm lazy, so we just wait the amount of time and check joining */
        NATIVECALL(nanosleep(abstime, NULL));

        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            if (retval) {
                *retval = thread->retval;
            }
            ThreadManager::threadDetach(pthread_id);
        }
        else {
            ret = ETIMEDOUT;
        }
    }
    else {
        ret = orig::pthread_timedjoin_np(pthread_id, retval, abstime);
        if (ret == 0) {
            ThreadManager::threadDetach(pthread_id);
        }
    }

    if (ret == 0)
        debuglog(LCF_THREAD, "Joining thread successfully.");
    else
        debuglog(LCF_THREAD, "Call timed out before thread terminated.");

    ThreadSync::wrapperExecutionLockUnlock();
    return ETIMEDOUT;
}

static std::map<pthread_cond_t*, clockid_t>& getCondClock() {
    static std::map<pthread_cond_t*, clockid_t> condClocks;
    return condClocks;
}

/* Override */ int pthread_cond_init (pthread_cond_t *cond, const pthread_condattr_t *cond_attr) __THROW
{
    LINK_NAMESPACE_VERSION(pthread_cond_init, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_init(cond, cond_attr);

    debuglog(LCF_WAIT, __func__, " call with cond ", static_cast<void*>(cond));

    /* Store the clock if one is set for `pthread_cond_timedwait()` */
    if (cond_attr) {        
        clockid_t clock_id;
        LINK_NAMESPACE(pthread_condattr_getclock, "pthread");
        orig::pthread_condattr_getclock(cond_attr, &clock_id);
        
        std::map<pthread_cond_t*, clockid_t>& condClocks = getCondClock();
        condClocks[cond] = clock_id;
    }

    return orig::pthread_cond_init(cond, cond_attr);
}

/* Override */ int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    LINK_NAMESPACE_VERSION(pthread_cond_wait, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_wait(cond, mutex);

    if (shared_config.game_specific_sync & SharedConfig::GC_SYNC_CELESTE) {
        ThreadSync::detSignal(false);
    }

    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex));
    return orig::pthread_cond_wait(cond, mutex);
}

/* Override */ int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    LINK_NAMESPACE_VERSION(pthread_cond_timedwait, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_timedwait(cond, mutex, abstime);

    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond), " and mutex ", static_cast<void*>(mutex), " and timeout ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000, " ms.");

    /* Get clock_id of cond */
    clockid_t clock_id = CLOCK_REALTIME;
    const std::map<pthread_cond_t*, clockid_t>& condClocks = getCondClock();

    auto it = condClocks.find(cond);
    if (it != condClocks.end())
        clock_id = it->second;

    /* Convert the abstime variable because pthread_cond_timedwait() is using
     * the real system time. */
    struct timespec new_abstime = *abstime;
    TimeHolder abs_timeout = *abstime;
    TimeHolder real_time;
    NATIVECALL(clock_gettime(clock_id, &real_time));
    TimeHolder rel_timeout = abs_timeout - real_time;
    /* We presume that the game won't call pthread_cond_timedwait() with
     * a negative time, or more than 10 seconds.
     */
    if ((rel_timeout.tv_sec < -1) || (rel_timeout.tv_sec > 10)) {
        /* Change the reference time to real system time */
        TimeHolder fake_time = detTimer.getTicks();
        TimeHolder new_rel_timeout = abs_timeout - fake_time;
        TimeHolder new_abs_timeout = real_time + new_rel_timeout;
        new_abstime = new_abs_timeout;
        debuglog(LCF_WAIT, " Rel time was ", 1000*new_rel_timeout.tv_sec + new_rel_timeout.tv_nsec/1000000, " ms.");
        debuglog(LCF_WAIT, " New abs time is ", new_abstime.tv_sec, ".", new_abstime.tv_nsec/1000000, " s");
    }

    /* If not main thread, do not change the behavior */
    if (!ThreadManager::isMainThread())
        return orig::pthread_cond_timedwait(cond, mutex, &new_abstime);

    if (shared_config.wait_timeout == SharedConfig::WAIT_NATIVE)
        return orig::pthread_cond_timedwait(cond, mutex, &new_abstime);

    if (shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait for 0.1 sec, arbitrary */
        TimeHolder delta_time;
        delta_time.tv_sec = 0;
        delta_time.tv_nsec = 100*1000*1000;
        TimeHolder new_end_time = real_time + delta_time;
        int ret = orig::pthread_cond_timedwait(cond, mutex, &new_end_time);
        if (ret == 0)
            return ret;
    }

    if ((shared_config.wait_timeout == SharedConfig::WAIT_FULL_INFINITE) ||
        (shared_config.wait_timeout == SharedConfig::WAIT_FINITE) ||
        (shared_config.wait_timeout == SharedConfig::WAIT_FULL))
        {
        /* Transfer time to our deterministic timer */
        TimeHolder now = detTimer.getTicks();
        TimeHolder delay = abs_timeout - now;
        detTimer.addDelay(delay);
    }

    if (shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait again for 0.1 sec, arbitrary */
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &real_time));
        TimeHolder delta_time;
        delta_time.tv_sec = 0;
        delta_time.tv_nsec = 100*1000*1000;
        TimeHolder new_end_time = real_time + delta_time;
        return orig::pthread_cond_timedwait(cond, mutex, &new_end_time);
    }

    if ((shared_config.wait_timeout == SharedConfig::NO_WAIT) ||
        (shared_config.wait_timeout == SharedConfig::WAIT_FULL)) {
        return orig::pthread_cond_timedwait(cond, mutex, &real_time);
    }

    /* Infinite wait */
    LINK_NAMESPACE_VERSION(pthread_cond_wait, "pthread", "GLIBC_2.3.2");
    return orig::pthread_cond_wait(cond, mutex);
}

/* Override */ int pthread_cond_signal(pthread_cond_t *cond) __THROW
{
    LINK_NAMESPACE_VERSION(pthread_cond_signal, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_signal(cond);

    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with cond ", static_cast<void*>(cond));
    return orig::pthread_cond_signal(cond);
}

/* Override */ int pthread_cond_broadcast(pthread_cond_t *cond) __THROW
{
    LINK_NAMESPACE_VERSION(pthread_cond_broadcast, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_broadcast(cond);

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

// /* Override */ int sem_wait (sem_t *sem)
// {
//     LINK_NAMESPACE(sem_wait, "pthread");
//     if (GlobalState::isNative())
//         return orig::sem_wait(sem);
//
//     debuglogstdio(LCF_THREAD | LCF_WAIT, "sem_wait call with %p", sem);
//     return orig::sem_wait(sem);
// }

/* Override */ int sem_timedwait (sem_t * sem, const struct timespec *abstime)
{
    LINK_NAMESPACE(sem_timedwait, "pthread");
    if (GlobalState::isNative())
        return orig::sem_timedwait(sem, abstime);

    debuglog(LCF_WAIT | LCF_TODO, __func__, " call with sem ", static_cast<void*>(sem), " and timeout ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000, " ms.");

    /* Convert the abstime variable because sem_timedwait() is using
     * the real system time. */
    struct timespec new_abstime = *abstime;
    TimeHolder abs_timeout = *abstime;
    TimeHolder real_time;
    NATIVECALL(clock_gettime(CLOCK_REALTIME, &real_time));
    TimeHolder rel_timeout = abs_timeout - real_time;
    /* We presume that the game won't call sem_timedwait() with
     * a negative time, or more than 10 seconds.
     */
    if ((rel_timeout.tv_sec < -1) || (rel_timeout.tv_sec > 10)) {
        /* Change the reference time to real system time */
        TimeHolder fake_time = detTimer.getTicks();
        TimeHolder new_rel_timeout = abs_timeout - fake_time;
        TimeHolder new_abs_timeout = real_time + new_rel_timeout;
        new_abstime = new_abs_timeout;
        debuglog(LCF_WAIT, " Rel time was ", 1000*new_rel_timeout.tv_sec + new_rel_timeout.tv_nsec/1000000, " ms.");
        debuglog(LCF_WAIT, " New abs time is ", new_abstime.tv_sec, ".", new_abstime.tv_nsec/1000000, " s");
    }

    return orig::sem_timedwait(sem, &new_abstime);
}

/* Override */ int sem_trywait (sem_t *sem) __THROW
{
    LINK_NAMESPACE(sem_trywait, "pthread");
    if (GlobalState::isNative())
        return orig::sem_trywait(sem);

    DEBUGLOGCALL(LCF_THREAD | LCF_WAIT | LCF_TODO);
    return orig::sem_trywait(sem);
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize) __THROW
{
    LINK_NAMESPACE(pthread_attr_setstack, "pthread");
    if (GlobalState::isNative())
        return orig::pthread_attr_setstack(attr, stackaddr, stacksize);

    debuglog(LCF_THREAD, __func__, " called with addr ", stackaddr, " and size ", stacksize);
    // int ret = orig::pthread_attr_setstack(attr, stackaddr, stacksize);
    // debuglog(LCF_THREAD, "  returns ", ret);
    // return ret;
    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id) __THROW
{
    LINK_NAMESPACE(pthread_condattr_setclock, "pthread");
    debuglogstdio(LCF_THREAD | LCF_WAIT, "%s called with clock %d", __func__, clock_id);

    int ret = orig::pthread_condattr_setclock(attr, clock_id);
    return ret;
}

int pthread_setname_np (pthread_t target_thread, const char *name) __THROW
{
    LINK_NAMESPACE(pthread_setname_np, "pthread");
    if (GlobalState::isNative())
        return orig::pthread_setname_np(target_thread, name);

    debuglog(LCF_THREAD, __func__, " called with target_thread ", target_thread, " and name ", name);

    /* Check if the thread is one of the llvm ones, and make it native and disable log */
    if (strncmp(name, "llvmpipe-", 9) == 0) {
        GlobalState::setNative(true);
        GlobalState::setNoLog(true);
    }

    if (shared_config.game_specific_sync & SharedConfig::GC_SYNC_CELESTE) {
        if ((strcmp(name, "OVERWORLD_LOADE") == 0) ||
            (strcmp(name, "LEVEL_LOADER") == 0) ||
            (strcmp(name, "USER_IO") == 0) ||
            (strcmp(name, "FILE_LOADING") == 0) ||
            (strcmp(name, "COMPLETE_LEVEL") == 0) ||
            (strcmp(name, "SUMMIT_VIGNETTE") == 0)) {
            ThreadSync::detInit();
        }
    }

    return orig::pthread_setname_np(target_thread, name);
}

}
