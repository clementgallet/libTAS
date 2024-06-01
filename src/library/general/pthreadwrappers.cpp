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

#include "pthreadwrappers.h"
#include "tlswrappers.h"
#include "timewrappers.h" // gettimeofday()

#include "logging.h"
#include "checkpoint/ThreadInfo.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadSync.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "global.h"
#include "GameHacks.h"
#include "GlobalState.h"
#include "UnityHacks.h"

#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <memory>
#include <exception>

namespace libtas {

DEFINE_ORIG_POINTER(pthread_create)
DEFINE_ORIG_POINTER(pthread_detach)
DEFINE_ORIG_POINTER(pthread_cond_init)
DEFINE_ORIG_POINTER(pthread_cond_wait)
DEFINE_ORIG_POINTER(pthread_cond_timedwait)
DEFINE_ORIG_POINTER(pthread_cond_signal)
DEFINE_ORIG_POINTER(pthread_cond_broadcast)
DEFINE_ORIG_POINTER(sem_wait)
DEFINE_ORIG_POINTER(sem_timedwait)
DEFINE_ORIG_POINTER(sem_trywait)
DEFINE_ORIG_POINTER(sem_post)
DEFINE_ORIG_POINTER(pthread_condattr_getclock)
DEFINE_ORIG_POINTER(pthread_setname_np)
#if defined(__APPLE__) && defined(__MACH__)
DEFINE_ORIG_POINTER(_pthread_workqueue_init_with_workloop)
/* Internal function */
int __pthread_workqueue_setkill(int enable);
DEFINE_ORIG_POINTER(__pthread_workqueue_setkill)
#endif

static void *pthread_start(void *arg)
{
    ThreadInfo *thread = static_cast<ThreadInfo*>(arg);

    ThreadManager::initThreadFromChild(thread);
    ThreadManager::setGlobalState(thread);
    ThreadSync::decrementUninitializedThreadCount();

    LOG(LL_DEBUG, LCF_THREAD, "Beginning of thread code %td", thread->routine_id);

    /* Execute the function */
    void *ret = thread->start(thread->arg);

    LOG(LL_DEBUG, LCF_THREAD, "End of thread code");

    WrapperLock wrapperLock;
    ThreadManager::threadExit(ret);

    return thread->retval;
}

static void *pthread_native_start(void *arg)
{
    ThreadInfo* thread = static_cast<ThreadInfo*>(arg);
    ThreadManager::setGlobalState(thread);

    auto thread_start = thread->start;
    auto thread_arg = thread->arg;
    delete thread;

    return thread_start(thread_arg);
}

/* Override */ int pthread_create (pthread_t * tid_p, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) __THROW
{
    LINK_NAMESPACE(pthread_create, "pthread");
    LINK_NAMESPACE(pthread_detach, "pthread");

    if (GlobalState::isNative()) {
        LOG(LL_DEBUG, LCF_THREAD, "Native Thread is created");
        ThreadInfo* thread = new ThreadInfo();
        ThreadManager::initThreadFromParent(thread, start_routine, arg, __builtin_return_address(0));
        return orig::pthread_create(tid_p, attr, pthread_native_start, thread);
    }

    LOG(LL_TRACE, LCF_THREAD, "Thread is created with routine %p", (void*)start_routine);

    WrapperLock wrapperLock;
    ThreadSync::incrementUninitializedThreadCount();

    /* Creating a new thread, and filling some parameters.
     * The rest (like thread->tid) will be filled by the child thread.
     */
    ThreadInfo* thread = new ThreadInfo;
    ThreadManager::initThreadFromParent(thread, start_routine, arg, __builtin_return_address(0));

    /* Threads can be created in detached state */
    int detachstate = PTHREAD_CREATE_JOINABLE; // default
    if (attr) {
        pthread_attr_getdetachstate(attr, &detachstate);
        if (detachstate != PTHREAD_CREATE_JOINABLE)
            LOG(LL_DEBUG, LCF_THREAD, "Detached state is ", detachstate);
    }
    thread->detached = (detachstate == PTHREAD_CREATE_DETACHED);

    /* Call our wrapper function */
    int ret = orig::pthread_create(tid_p, attr, pthread_start, thread);

    if (ret != 0) {
        /* Thread creation failed */
        ThreadSync::decrementUninitializedThreadCount();
        ThreadManager::threadIsDead(thread);
    }

    /* Immediatly detach the thread. We don't want to deal with zombie threads,
     * so we implement ourself the joinable state, and all threads are detached. */
    orig::pthread_detach (*tid_p);
    return ret;
}

/* Override */ void pthread_exit (void *retval)
{
    RETURN_IF_NATIVE(pthread_exit, (retval), "libpthread.so");

    LOG(LL_TRACE, LCF_THREAD, "Thread has exited.");

    ThreadManager::threadExit(retval);
    RETURN_NATIVE(pthread_exit, (retval), nullptr);
}

/* Override */ int pthread_join (pthread_t pthread_id, void **thread_return)
{
    RETURN_IF_NATIVE(pthread_join, (pthread_id, thread_return), "libpthread.so");

    ThreadSync::waitForThreadsToFinishInitialization();

    LOG(LL_TRACE, LCF_THREAD, "Joining thread id %p tid %d", pthread_id, ThreadManager::getThreadTid(pthread_id));

    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        return ESRCH;
    }

    if (thread->detached) {
        return EINVAL;
    }

    int ret = 0;
    /* Wait for the thread to become zombie */
    while (thread->state != ThreadInfo::ST_ZOMBIE) {
        struct timespec mssleep = {0, 1000*1000};
        NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
    }
    if (thread_return) {
        *thread_return = thread->retval;
    }

    WrapperLock wrapperLock;
    ThreadManager::threadDetach(pthread_id);
    return ret;
}

/* Override */ int pthread_detach (pthread_t pthread_id) __THROW
{
    LINK_NAMESPACE(pthread_detach, "libpthread.so");
    if (GlobalState::isNative()) {
        return orig::pthread_detach(pthread_id);
    }

    WrapperLock wrapperLock;
    ThreadSync::waitForThreadsToFinishInitialization();

    LOG(LL_TRACE, LCF_THREAD, "Detaching thread id %p tid %d", pthread_id, ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        return ESRCH;
    }

    if (thread->detached) {
        return EINVAL;
    }

    ThreadManager::threadDetach(pthread_id);
    return 0;
}

/* Override */ int pthread_tryjoin_np(pthread_t pthread_id, void **retval) __THROW
{
    RETURN_IF_NATIVE(pthread_tryjoin_np, (pthread_id, retval), "libpthread.so");

    WrapperLock wrapperLock;
    ThreadSync::waitForThreadsToFinishInitialization();

    LOG(LL_TRACE, LCF_THREAD, "Try to join thread %d", ThreadManager::getThreadTid(pthread_id));
    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        return ESRCH;
    }

    if (thread->detached) {
        return EINVAL;
    }

    int ret = 0;
    if (thread->state == ThreadInfo::ST_ZOMBIE) {
        if (retval) {
            *retval = thread->retval;
        }
        ThreadManager::threadDetach(pthread_id);
    }
    else {
        ret = EBUSY;
    }
    
    if (ret == 0)
        LOG(LL_DEBUG, LCF_THREAD, "Joining thread successfully.");
    else
        LOG(LL_DEBUG, LCF_THREAD, "Thread has not yet terminated.");
    return EBUSY;
}

/* Override */ int pthread_timedjoin_np(pthread_t pthread_id, void **retval, const struct timespec *abstime)
{
    RETURN_IF_NATIVE(pthread_timedjoin_np, (pthread_id, retval, abstime), "libpthread.so");

    WrapperLock wrapperLock;
    ThreadSync::waitForThreadsToFinishInitialization();

    LOG(LL_TRACE, LCF_THREAD | LCF_TODO, "Try to join thread in %d.%010d sec", abstime->tv_sec, abstime->tv_nsec);

    if (abstime->tv_sec < 0 || abstime->tv_nsec >= 1000000000) {
        return EINVAL;
    }

    ThreadInfo* thread = ThreadManager::getThread(pthread_id);

    if (!thread) {
        return ESRCH;
    }

    if (thread->detached) {
        return EINVAL;
    }

    int ret = 0;
    
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

    if (ret == 0)
        LOG(LL_DEBUG, LCF_THREAD, "Joining thread successfully.");
    else
        LOG(LL_DEBUG, LCF_THREAD, "Call timed out before thread terminated.");

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

    LOG(LL_TRACE, LCF_WAIT, "%s call with cond %p", __func__, static_cast<void*>(cond));

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

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_CELESTE) {
        ThreadSync::detSignal(false);
    }

    LOG(LL_TRACE, LCF_WAIT | LCF_TODO, "%s call with cond %p and mutex %p", __func__, static_cast<void*>(cond), static_cast<void*>(mutex));
    return orig::pthread_cond_wait(cond, mutex);
}

/* Override */ int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    LINK_NAMESPACE_VERSION(pthread_cond_timedwait, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_timedwait(cond, mutex, abstime);

    LOG(LL_TRACE, LCF_WAIT | LCF_TODO, "%s call with cond %p and mutex %p and timeout %d.%010d sec", __func__, static_cast<void*>(cond), static_cast<void*>(mutex), abstime->tv_sec, abstime->tv_nsec);

    /* Convert the abstime variable because pthread_cond_timedwait() is using
     * the real system time. */
    struct timespec new_abstime = *abstime;
    TimeHolder abs_timeout = *abstime;
    TimeHolder real_time;
    SharedConfig::TimeCallType time_type = SharedConfig::TIMETYPE_UNTRACKED_REALTIME;
    
#ifdef __unix__
    /* Get clock_id of cond */
    clockid_t clock_id = CLOCK_REALTIME;
    const std::map<pthread_cond_t*, clockid_t>& condClocks = getCondClock();

    auto it = condClocks.find(cond);
    if (it != condClocks.end())
        clock_id = it->second;

    NATIVECALL(clock_gettime(clock_id, &real_time));
    time_type = DeterministicTimer::get().clockToType(clock_id);

#elif defined(__APPLE__) && defined(__MACH__)
    /* On MacOS, the reference time is based on gettimeofday(). */
    timeval tv;
    NATIVECALL(gettimeofday(&tv, nullptr));
    real_time.tv_sec = tv.tv_sec;
    real_time.tv_nsec = tv.tv_usec * 1000;
#endif

    TimeHolder rel_timeout = abs_timeout - real_time;
    /* We presume that the game won't call pthread_cond_timedwait() with
     * a negative time, or more than 10 seconds. */
    if ((rel_timeout.tv_sec < -1) || (rel_timeout.tv_sec > 10)) {
        /* Change the reference time to real system time */
        TimeHolder fake_time = DeterministicTimer::get().getTicks(time_type);
        TimeHolder new_rel_timeout = abs_timeout - fake_time;
        TimeHolder new_abs_timeout = real_time + new_rel_timeout;
        new_abstime = new_abs_timeout;
        LOG(LL_DEBUG, LCF_WAIT, " Rel time was %d.%010d sec", new_rel_timeout.tv_sec, new_rel_timeout.tv_nsec);
        LOG(LL_DEBUG, LCF_WAIT, " New abs time is %d.%010d sec", new_abstime.tv_sec, new_abstime.tv_nsec);
    }

    /* If not main thread, do not change the behavior */
    if (!ThreadManager::isMainThread()) {
        int ret = orig::pthread_cond_timedwait(cond, mutex, &new_abstime);
        LOG(LL_DEBUG, LCF_WAIT, "   ret is %d ", ret);
        return ret;
    }

    /* When game is exiting, we want time to advance to prevent softlocks */
    if (Global::is_exiting) {
        /* Transfer time to our deterministic timer */
        TimeHolder now = DeterministicTimer::get().getTicks(time_type);
        TimeHolder delay = abs_timeout - now;
        DeterministicTimer::get().addDelay(delay);

        return orig::pthread_cond_timedwait(cond, mutex, &real_time);
    }

    if (Global::shared_config.wait_timeout == SharedConfig::WAIT_NATIVE)
        return orig::pthread_cond_timedwait(cond, mutex, &new_abstime);

    if (Global::shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait for 0.1 sec, arbitrary */
        TimeHolder delta_time;
        delta_time.tv_sec = 0;
        delta_time.tv_nsec = 100*1000*1000;
        TimeHolder new_end_time = real_time + delta_time;
        int ret = orig::pthread_cond_timedwait(cond, mutex, &new_end_time);
        if (ret == 0)
            return ret;
    }

    if ((Global::shared_config.wait_timeout == SharedConfig::WAIT_FULL_INFINITE) ||
        (Global::shared_config.wait_timeout == SharedConfig::WAIT_FINITE) ||
        (Global::shared_config.wait_timeout == SharedConfig::WAIT_FULL))
        {
        /* Transfer time to our deterministic timer */
        TimeHolder now = DeterministicTimer::get().getTicks(time_type);
        TimeHolder delay = abs_timeout - now;
        DeterministicTimer::get().addDelay(delay);
    }

    if (Global::shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait again for 0.1 sec, arbitrary */
#ifdef __unix__
        NATIVECALL(clock_gettime(clock_id, &real_time));
#elif defined(__APPLE__) && defined(__MACH__)
        timeval tv;
        NATIVECALL(gettimeofday(&tv, nullptr));
        real_time.tv_sec = tv.tv_sec;
        real_time.tv_nsec = tv.tv_usec * 1000;
#endif
        TimeHolder delta_time;
        delta_time.tv_sec = 0;
        delta_time.tv_nsec = 100*1000*1000;
        TimeHolder new_end_time = real_time + delta_time;
        return orig::pthread_cond_timedwait(cond, mutex, &new_end_time);
    }

    if ((Global::shared_config.wait_timeout == SharedConfig::NO_WAIT) ||
        (Global::shared_config.wait_timeout == SharedConfig::WAIT_FULL)) {
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

    LOG(LL_TRACE, LCF_WAIT | LCF_TODO, "%s call with cond %p", __func__, static_cast<void*>(cond));
    return orig::pthread_cond_signal(cond);
}

/* Override */ int pthread_cond_broadcast(pthread_cond_t *cond) __THROW
{
    LINK_NAMESPACE_VERSION(pthread_cond_broadcast, "pthread", "GLIBC_2.3.2");
    if (GlobalState::isNative())
        return orig::pthread_cond_broadcast(cond);

    LOG(LL_TRACE, LCF_WAIT | LCF_TODO, "%s call with cond %p", __func__, static_cast<void*>(cond));
    return orig::pthread_cond_broadcast(cond);
}

/* Override */ int pthread_setcancelstate (int state, int *oldstate)
{
    RETURN_IF_NATIVE(pthread_setcancelstate, (state, oldstate), "libpthread.so");
    LOGTRACE(LCF_THREAD | LCF_TODO);
    RETURN_NATIVE(pthread_setcancelstate, (state, oldstate), "libpthread.so");
}

/* Override */ int pthread_setcanceltype (int type, int *oldtype)
{
    RETURN_IF_NATIVE(pthread_setcanceltype, (type, oldtype), "libpthread.so");
    LOGTRACE(LCF_THREAD | LCF_TODO);
    RETURN_NATIVE(pthread_setcanceltype, (type, oldtype), "libpthread.so");
}

/* Override */ int pthread_cancel (pthread_t pthread_id)
{
    RETURN_IF_NATIVE(pthread_cancel, (pthread_id), "libpthread.so");
    LOG(LL_TRACE, LCF_THREAD | LCF_TODO, "Cancel thread %d", ThreadManager::getThreadTid(pthread_id));
    RETURN_NATIVE(pthread_cancel, (pthread_id), "libpthread.so");
}

/* Override */ void pthread_testcancel (void)
{
    RETURN_IF_NATIVE(pthread_testcancel, (), "libpthread.so");
    LOGTRACE(LCF_THREAD | LCF_TODO);
    RETURN_NATIVE(pthread_testcancel, (), "libpthread.so");
}

/* Override */ int sem_wait (sem_t *sem)
{
    LINK_NAMESPACE_VERSION(sem_wait, "pthread", "GLIBC_2.1");
    if (GlobalState::isNative())
        return orig::sem_wait(sem);

    ThreadInfo* thread = ThreadManager::getCurrentThread();
    bool isWaitThread = UnityHacks::isLoadingThread(reinterpret_cast<uintptr_t>(thread->start));

    LOG(LL_TRACE, LCF_WAIT, "sem_wait call with %p", sem);
    if (isWaitThread) {
        UnityHacks::syncNotify();

        int ret = orig::sem_wait(sem);

        UnityHacks::syncWait();
        return ret;
    }

    return orig::sem_wait(sem);
}

/* Override */ int sem_timedwait (sem_t * sem, const struct timespec *abstime)
{
    RETURN_IF_NATIVE(sem_timedwait, (sem, abstime), "libpthread.so");

    LOG(LL_TRACE, LCF_WAIT | LCF_TODO, "%s call with sem %p and timeout %d.%010d sec", __func__, static_cast<void*>(sem), abstime->tv_sec, abstime->tv_nsec);

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
        TimeHolder fake_time = DeterministicTimer::get().getTicks();
        TimeHolder new_rel_timeout = abs_timeout - fake_time;
        TimeHolder new_abs_timeout = real_time + new_rel_timeout;
        new_abstime = new_abs_timeout;
        LOG(LL_DEBUG, LCF_WAIT, " Rel time was %d.%010d sec", new_rel_timeout.tv_sec, new_rel_timeout.tv_nsec);
        LOG(LL_DEBUG, LCF_WAIT, " New abs time is %d.%010d sec", new_abstime.tv_sec, new_abstime.tv_nsec);
    }

    RETURN_NATIVE(sem_timedwait, (sem, &new_abstime), "libpthread.so");
}

/* Override */ int sem_trywait (sem_t *sem) __THROW
{
    LINK_NAMESPACE_VERSION(sem_trywait, "pthread", "GLIBC_2_1");
    if (GlobalState::isNative())
        return orig::sem_trywait(sem);

    LOGTRACE(LCF_WAIT | LCF_TODO);
    return orig::sem_trywait(sem);
}

/* Override */ int sem_post (sem_t *sem) __THROW
{
    LINK_NAMESPACE_VERSION(sem_post, "pthread", "GLIBC_2.1");
    if (GlobalState::isNative())
        return orig::sem_post(sem);

    LOG(LL_TRACE, LCF_WAIT, "%s called with sem %p", __func__, sem);
    return orig::sem_post(sem);
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize) __THROW
{
    RETURN_IF_NATIVE(pthread_attr_setstack, (attr, stackaddr, stacksize), "libpthread.so");

    LOG(LL_TRACE, LCF_THREAD, "%s called with addr %p and size %d", __func__, stackaddr, stacksize);
    // int ret = orig::pthread_attr_setstack(attr, stackaddr, stacksize);
    // LOG(LL_DEBUG, LCF_THREAD, "  returns %d", ret);
    // return ret;
    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id) __THROW
{
    LOG(LL_TRACE, LCF_THREAD | LCF_WAIT, "%s called with clock %d", __func__, clock_id);
    RETURN_NATIVE(pthread_condattr_setclock, (attr, clock_id), "libpthread.so");    
}

#ifdef __unix__

int pthread_setname_np (pthread_t target_thread, const char *name) __THROW
{
    LINK_NAMESPACE(pthread_setname_np, "pthread");
    if (GlobalState::isNative())
        return orig::pthread_setname_np(target_thread, name);
    
    LOG(LL_TRACE, LCF_THREAD, "%s called with target_thread %p and name %s", __func__, target_thread, name);

    /* Save name for debugging */
    ThreadInfo* thread = ThreadManager::getThread(target_thread);
    if (thread) {
        thread->name = name;
    }

#elif defined(__APPLE__) && defined(__MACH__)

int pthread_setname_np (const char *name)
{
    LINK_NAMESPACE(pthread_setname_np, "pthread");
    if (GlobalState::isNative())
        return orig::pthread_setname_np(name);

    LOG(LL_TRACE, LCF_THREAD, "%s called with name %s", __func__, name);

    /* Save name for debugging */
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    if (thread) {
        thread->name = name;
    }

#endif

    /* Check if the thread is one of the llvm ones, and make it native and disable log */
    if (strncmp(name, "llvmpipe-", 9) == 0) {
        GlobalState::setNative(true);
        GlobalState::setNoLog(true);
    }
    if (strstr(name, ":disk$")) {
        GlobalState::setNative(true);
        GlobalState::setNoLog(true);
    }

    if (strcmp(name, ".NET Finalizer") == 0) {
#ifdef __unix__
        GameHacks::setFinalizerThread(ThreadManager::getThreadTid(target_thread));
#elif defined(__APPLE__) && defined(__MACH__)
        GameHacks::setFinalizerThread(ThreadManager::getThreadTid());
#endif
    }
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_CELESTE) {
        if ((strcmp(name, "OVERWORLD_LOADE") == 0) ||
            (strcmp(name, "LEVEL_LOADER") == 0) ||
            (strcmp(name, "USER_IO") == 0) ||
            (strcmp(name, "FILE_LOADING") == 0) ||
            (strcmp(name, "COMPLETE_LEVEL") == 0) ||
            (strcmp(name, "SUMMIT_VIGNETTE") == 0)) {
            ThreadSync::detInit();
        }
    }

    if (Global::shared_config.game_specific_timing & SharedConfig::GC_TIMING_ARMA_CWA) {
        if (strcmp(name, "G.Main") == 0) {
            ThreadManager::setMainThread(target_thread);
        }
    }

#ifdef __unix__
    UnityHacks::waitFromName(target_thread, name);
#endif

#ifdef __unix__
    return orig::pthread_setname_np(target_thread, name);
#elif defined(__APPLE__) && defined(__MACH__)
    return orig::pthread_setname_np(name);
#endif
}

#if defined(__APPLE__) && defined(__MACH__)
static void register_self_thread()
{
    /* Exit if thread already registered */
    if (ThreadManager::getThreadTid() != 0)
        return;
    
    ThreadSync::incrementUninitializedThreadCount();
    
    /* Creating a new thread and filling parameters. */
    ThreadInfo* thread = new ThreadInfo;
    ThreadManager::initThreadFromParent(thread, nullptr, nullptr, __builtin_return_address(0));
    
    thread->detached = false;
    ThreadManager::initThreadFromChild(thread);
    
    /* Flag thread as killable */
    LINK_NAMESPACE(__pthread_workqueue_setkill, "system_pthread");
    orig::__pthread_workqueue_setkill(1);
    
    ThreadSync::decrementUninitializedThreadCount();
}
    
namespace orig {
    static pthread_workqueue_function2_t queue_func;
    static pthread_workqueue_function_kevent_t kevent_func;
    static pthread_workqueue_function_workloop_t workloop_func;
}

static void pthread_workqueue_queue_func(pthread_priority_t priority)
{
    /* Register own thread if not already */
    register_self_thread();
    
    /* Returns original function */
    return orig::queue_func(priority);
}

static void pthread_workqueue_kevent_func(void **events, int *nevents)
{
    /* Register own thread if not already */
    register_self_thread();
    
    /* Returns original function */
    return orig::kevent_func(events, nevents);
}

static void pthread_workqueue_workloop_func(uint64_t *workloop_id, void **events, int *nevents)
{
    /* Register own thread if not already */
    register_self_thread();
    
    /* Returns original function */
    return orig::workloop_func(workloop_id, events, nevents);
}

int _pthread_workqueue_init_with_workloop(pthread_workqueue_function2_t queue_func, pthread_workqueue_function_kevent_t kevent_func, pthread_workqueue_function_workloop_t workloop_func, int offset, int flags)
{
    LOGTRACE(LCF_THREAD);

    /* Save orig workqueue functions */
    orig::queue_func = queue_func;
    orig::kevent_func = kevent_func;
    orig::workloop_func = workloop_func;

    LINK_NAMESPACE(_pthread_workqueue_init_with_workloop, "system_pthread");
    return orig::_pthread_workqueue_init_with_workloop(pthread_workqueue_queue_func,  pthread_workqueue_kevent_func,  pthread_workqueue_workloop_func, offset, flags);
}

#endif

}
