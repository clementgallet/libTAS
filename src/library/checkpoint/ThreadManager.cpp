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
#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include <sys/syscall.h> // syscall, SYS_gettid
#include <pthread.h>

#include "ThreadManager.h"
#include "SaveStateManager.h"
#include "Checkpoint.h"
#include "ThreadSync.h"
#include "../timewrappers.h" // clock_gettime
#include "../logging.h"
#include "../hook.h"
//#include "../audio/AudioPlayer.h"
#include "AltStack.h"
#include "ReservedMemory.h"
#include "../fileio/FileHandleList.h"

namespace libtas {

ThreadInfo* ThreadManager::thread_list = nullptr;
thread_local ThreadInfo* ThreadManager::current_thread = nullptr;
pthread_t ThreadManager::main_pthread_id = 0;
pthread_mutex_t ThreadManager::threadStateLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadManager::threadListLock = PTHREAD_MUTEX_INITIALIZER;
bool ThreadManager::is_child_fork = false;

#ifdef __i386__
int ThreadManager::offset_tid = 26;
#elif __x86_64__
int ThreadManager::offset_tid = 180;
#else
int ThreadManager::offset_tid = 0;
#endif



void ThreadManager::init()
{
    /* Create a ThreadInfo struct for this thread */
    ThreadInfo* thread = ThreadManager::getNewThread();
    thread->state = ThreadInfo::ST_RUNNING;
    thread->detached = false;
    initThreadFromChild(thread);

    setMainThread();

    /* Identify tid offset in pthread structure */
    int* thread_data = reinterpret_cast<int*>(thread->pthread_id);
    if (thread_data[offset_tid] != thread->tid) {
        for (offset_tid = 0; offset_tid < 500; offset_tid++) {
            if (thread_data[offset_tid] == thread->tid) {
                break;
            }
        }
        if (offset_tid == 500) {
            debuglogstdio(LCF_THREAD | LCF_ERROR, "Could not find tid member in pthread!");
        }
    }
}

DEFINE_ORIG_POINTER(pthread_self)

pthread_t ThreadManager::getThreadId()
{
    LINK_NAMESPACE(pthread_self, "pthread");
    if (orig::pthread_self != nullptr)
        return orig::pthread_self();

    /* We couldn't link to pthread, meaning threading should be off.
     * We must return a value so that isMainThread() returns true.
     */
    return main_pthread_id;
}

pid_t ThreadManager::getThreadTid()
{
    /* MacOS cannot access TLS at startup */
#if defined(__APPLE__) && defined(__MACH__)
    if (!is_inited)
        return 0;
#endif

    if (current_thread)
        return current_thread->tid;
    return 0;
}

void ThreadManager::setMainThread()
{
    pthread_t pthread_id = getThreadId();

    if (!main_pthread_id) {
        main_pthread_id = pthread_id;
        return;
    }

    if (main_pthread_id != pthread_id) {
        if (!(shared_config.debug_state & SharedConfig::DEBUG_MAIN_FIRST_THREAD)) {
            /* Switching main thread */
            debuglogstdio(LCF_THREAD | LCF_WARNING, "Switching main thread from %d to %d", main_pthread_id, pthread_id);
            main_pthread_id = pthread_id;
        }
    }
}

void ThreadManager::setCheckpointThread()
{
    if (current_thread->state == ThreadInfo::ST_CKPNTHREAD)
        return;

    /* Remove state of old checkpoint thread */
    lockList();

    for (ThreadInfo* th = thread_list; th != nullptr; th = th->next) {
        if (th->state == ThreadInfo::ST_CKPNTHREAD) {
            th->state = ThreadInfo::ST_RUNNING;
        }
    }

    current_thread->state = ThreadInfo::ST_CKPNTHREAD;
    SaveStateManager::initCheckpointThread();

    unlockList();
}

bool ThreadManager::isMainThread()
{
    /* Check if main thread has been set */
    if (main_pthread_id) {
        return (getThreadId() == main_pthread_id);
    }

    return true;
}

ThreadInfo* ThreadManager::getNewThread()
{
    ThreadInfo* thread = nullptr;

    lockList();
    /* Try to recycle a free thread */
    for (ThreadInfo* th = thread_list; th != nullptr; th = th->next) {
        if (th->state == ThreadInfo::ST_FREE) {
            thread = th;
            /* We must change the state here so that this thread is not chosen
             * twice by two different threads.
             */
            thread->state = ThreadInfo::ST_RECYCLED;
            break;
        }
    }

    /* No free thread, create a new one */
    if (!thread) {
        thread = new ThreadInfo;
        debuglogstdio(LCF_THREAD, "Allocate a new ThreadInfo struct");
        threadListChanged = true;
    }

    unlockList();
    return thread;
}

ThreadInfo* ThreadManager::getThread(pthread_t pthread_id)
{
    for (ThreadInfo* thread = thread_list; thread != nullptr; thread = thread->next)
        if (thread->pthread_id == pthread_id)
            return thread;

    return nullptr;
}

pid_t ThreadManager::getThreadTid(pthread_t pthread_id)
{
    ThreadInfo* ti = getThread(pthread_id);
    if (ti)
        return ti->tid;
    return 0;
}

void ThreadManager::restoreThreadTids()
{
    for (ThreadInfo* thread = thread_list; thread != nullptr; thread = thread->next) {
        int* thread_data = reinterpret_cast<int*>(thread->pthread_id);
        thread_data[offset_tid] = thread->tid;
    }
}

bool ThreadManager::initThreadFromParent(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from)
{
    debuglogstdio(LCF_THREAD, "Init thread with routine %p", (void*)start_routine);
    bool isRecycled = thread->state == ThreadInfo::ST_RECYCLED;

    thread->start = start_routine;
    thread->arg = arg;
    thread->state = ThreadInfo::ST_RUNNING;
    thread->routine_id = (char *)start_routine - (char *)from;
    thread->detached = false;
    thread->initial_native = GlobalState::isNative();
    thread->initial_owncode = GlobalState::isOwnCode();
    thread->initial_nolog = GlobalState::isNoLog();

    thread->syncCount = 0;
    thread->syncOldCount = 0;
    
    if (!isRecycled) {
        thread->pthread_id = 0;
        thread->tid = 0;

        thread->next = nullptr;
        thread->prev = nullptr;
    }

    return isRecycled;
}

void ThreadManager::initThreadFromChild(ThreadInfo* thread)
{
    thread->pthread_id = getThreadId();
    
#ifdef __unix__
    thread->tid = syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    thread->tid = tid;
#endif


    current_thread = thread;
    addToList(thread);

    SaveStateManager::initThreadFromChild(thread);
}

void ThreadManager::update(ThreadInfo* thread)
{
    /* If a thread that is in native state is creating another thread, we
     * consider that the entire new thread is in native mode (e.g. audio thread)
     */
    if (thread->initial_native) GlobalState::setNative(true);
    if (thread->initial_owncode) GlobalState::setOwnCode(true);
    if (thread->initial_nolog) GlobalState::setNoLog(true);
}

void ThreadManager::addToList(ThreadInfo* thread)
{
    lockList();

    /* Check for a thread with the same tid. If it is the same thread then we
     * have nothing to do. Otherwise, remove it.
     */
    ThreadInfo* cur_thread = getThread(thread->pthread_id);
    if (cur_thread) {
        if (thread == cur_thread) {
            unlockList();
            return;
        }
        threadIsDead(cur_thread);
    }

    /* Add the new thread to the list */
    thread->next = thread_list;
    thread->prev = nullptr;
    if (thread_list != nullptr) {
        thread_list->prev = thread;
    }
    thread_list = thread;

    unlockList();
}

bool ThreadManager::isChildFork()
{
    return is_child_fork;
}

void ThreadManager::setChildFork()
{
    is_child_fork = true;
}

void ThreadManager::threadIsDead(ThreadInfo *thread)
{
    debuglogstdio(LCF_THREAD, "Remove thread %d from list", thread->tid);

    if (thread->prev != nullptr) {
        thread->prev->next = thread->next;
    }
    if (thread->next != nullptr) {
        thread->next->prev = thread->prev;
    }
    if (thread == thread_list) {
        thread_list = thread_list->next;
    }

    if (thread->altstack.ss_sp) {
        free(thread->altstack.ss_sp);
    }
    delete(thread);

    threadListChanged = true;
}

void ThreadManager::threadDetach(pthread_t pthread_id)
{
    ThreadInfo* thread = getThread(pthread_id);

    if (thread) {
        lockList();
        thread->detached = true;
        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            debuglogstdio(LCF_THREAD, "Zombie thread %d is detached", thread->tid);
            MYASSERT(updateState(thread, ThreadInfo::ST_FREE, ThreadInfo::ST_ZOMBIE))
            if (! shared_config.recycle_threads) {
                threadIsDead(thread);
            }
        }
        unlockList();
    }
}

void ThreadManager::threadExit(void* retval)
{
    ThreadSync::detSignal(true);

    lockList();
    current_thread->retval = retval;
    MYASSERT(updateState(current_thread, ThreadInfo::ST_ZOMBIE, ThreadInfo::ST_RUNNING) ||
             updateState(current_thread, ThreadInfo::ST_ZOMBIE, ThreadInfo::ST_CKPNTHREAD))
    if (current_thread->detached) {
        debuglogstdio(LCF_THREAD, "Detached thread %d exited", current_thread->tid);
        MYASSERT(updateState(current_thread, ThreadInfo::ST_FREE, ThreadInfo::ST_ZOMBIE))
        if (! shared_config.recycle_threads) {
            threadIsDead(current_thread);
        }
    }
    unlockList();
}

void ThreadManager::deallocateThreads()
{
    if (isChildFork())
        return;

    lockList();

    ThreadInfo *next;
    for (ThreadInfo* thread = thread_list; thread != nullptr; thread = next) {
        next = thread->next;

        /* Skip the main thread */
        if (thread->state == ThreadInfo::ST_CKPNTHREAD)
            continue;

        /* Notify each thread to quit */
        thread->quit = true;
        thread->cv.notify_all();

        /* Join thread */
        NATIVECALL(pthread_detach(thread->pthread_id));

        /* Delete the thread struct */
        threadIsDead(thread);
    }
    unlockList();
}

bool ThreadManager::updateState(ThreadInfo *th, ThreadInfo::ThreadState newval, ThreadInfo::ThreadState oldval)
{
    bool res = false;

    MYASSERT(pthread_mutex_lock(&threadStateLock) == 0)
    if (oldval == th->state) {
        th->state = newval;
        res = true;
    }
    MYASSERT(pthread_mutex_unlock(&threadStateLock) == 0)
    return res;
}

void ThreadManager::lockList()
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
}

void ThreadManager::unlockList()
{
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}


}
