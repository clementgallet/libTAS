/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ThreadManager.h"
#include "SaveStateManager.h"
#include "ThreadSync.h"
#include "MemArea.h"
#ifdef __unix__
#include "ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "MachVmMaps.h"
#endif

#include "logging.h"
#include "hook.h"
#include "global.h"
#include "GlobalState.h"

#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include <unistd.h> // syscall
#include <sys/syscall.h> // SYS_gettid
#include <pthread.h>

namespace libtas {

static ThreadInfo* thread_list = nullptr;
static thread_local ThreadInfo* current_thread = nullptr;
static pthread_t main_pthread_id = 0;
static pthread_mutex_t threadStateLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t threadListLock = PTHREAD_MUTEX_INITIALIZER;
static bool is_child_fork = false;

void ThreadManager::init()
{
    /* Create a ThreadInfo struct for this thread */
    ThreadInfo* thread = new ThreadInfo;
    thread->state = ThreadInfo::ST_RUNNING;
    thread->detached = false;
    initThreadFromChild(thread);

    setMainThread();
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
    if (!Global::is_inited)
        return 0;
#endif

    if (current_thread)
        return current_thread->real_tid;
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
        if (!(Global::shared_config.debug_state & SharedConfig::DEBUG_MAIN_FIRST_THREAD) &&
            !(Global::shared_config.game_specific_timing & SharedConfig::GC_TIMING_ARMA_CWA)) {
            /* Switching main thread */
            LOG(LL_WARN, LCF_THREAD, "Switching main thread from %d to %d", main_pthread_id, pthread_id);
            main_pthread_id = pthread_id;
        }
    }
}

void ThreadManager::setMainThread(pthread_t pthread_id)
{
    main_pthread_id = pthread_id;
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
        return ti->real_tid;
    return 0;
}

void ThreadManager::restoreTid()
{
#ifdef __unix__
    current_thread->real_tid = syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    current_thread->real_tid = tid;
#endif

    pid_t* thread_data = reinterpret_cast<pid_t*>(current_thread->pthread_id);
    thread_data[getTidOffset()] = current_thread->real_tid;
}

void ThreadManager::initThreadFromParent(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from)
{
    LOG(LL_DEBUG, LCF_THREAD, "Init thread with routine %p", (void*)start_routine);

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
    
    thread->pthread_id = 0;
    thread->real_tid = 0;
    thread->translated_tid = 0;

    thread->next = nullptr;
    thread->prev = nullptr;
}

int ThreadManager::getTidOffset() {

    static int offset_tid = -1;
    
    if (offset_tid != -1)
        return offset_tid;

    /* Offset of `tid` member in the hidden `pthread` structure */
#ifdef __x86_64__
    offset_tid = 180;
#else
    offset_tid = 26;
#endif

    /* Identify tid offset in pthread structure */
    pid_t tid = current_thread->real_tid;    
    pid_t* thread_data = reinterpret_cast<pid_t*>(current_thread->pthread_id);
    if (thread_data[offset_tid] != tid) {
        for (offset_tid = 0; offset_tid < 500; offset_tid++) {
            if (thread_data[offset_tid] == tid) {
                break;
            }
        }
        if (offset_tid == 500) {
            LOG(LL_ERROR, LCF_THREAD, "Could not find tid member in pthread!");
        }
    }

    return offset_tid;
}

void ThreadManager::initThreadFromChild(ThreadInfo* thread)
{
    current_thread = thread;

    thread->pthread_id = getThreadId();
    
#ifdef __unix__
    thread->real_tid = syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    thread->real_tid = tid;
#endif
    thread->translated_tid = thread->real_tid;

    thread->ptid = ((pid_t*)thread->pthread_id) + getTidOffset();

    /* Get stack address */
    pthread_attr_t attrs;
    pthread_getattr_np(thread->pthread_id, &attrs);
    pthread_attr_getstack(&attrs, &thread->stack_addr, &thread->stack_size);

    addToList(thread);

    SaveStateManager::initThreadFromChild(thread);
}

void ThreadManager::setGlobalState(ThreadInfo* thread)
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

ThreadInfo* ThreadManager::getThreadList()
{
    return thread_list;
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
    LOG(LL_DEBUG, LCF_THREAD, "Remove thread %d from list", thread->real_tid);

    if (thread->prev != nullptr) {
        thread->prev->next = thread->next;
    }
    if (thread->next != nullptr) {
        thread->next->prev = thread->prev;
    }
    if (thread == thread_list) {
        thread_list = thread_list->next;
    }

    delete(thread);
}

void ThreadManager::threadDetach(pthread_t pthread_id)
{
    ThreadInfo* thread = getThread(pthread_id);

    if (thread) {
        lockList();
        thread->detached = true;
        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            LOG(LL_DEBUG, LCF_THREAD, "Zombie thread %d is detached", thread->real_tid);
            threadIsDead(thread);
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
        LOG(LL_DEBUG, LCF_THREAD, "Detached thread %d exited", current_thread->real_tid);
        threadIsDead(current_thread);
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

ThreadInfo *ThreadManager::getCurrentThread()
{
    return current_thread;
}

void ThreadManager::setCurrentThread(ThreadInfo *thread) {
    current_thread = thread;
}

void ThreadManager::lockList()
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
}

void ThreadManager::unlockList()
{
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::updateStackInfo()
{
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    Area area;
    bool not_eof = memMapLayout.getNextArea(&area);

    lockList();

    while (not_eof) {
        
        /* Look for non-rw areas that intersect a thread stack.
         * FNA split some thread stacks with a non-rw section at the end, which
         * makes padding fail, see an exemple of stack layout:
         *     7fffb2c00000-7fffb2c01000 rw-p 00000000 00:00 0
         *     7fffb2c01000-7fffb2c09000 ---p 00000000 00:00 0
         *     7fffb2c09000-7fffb2f00000 rw-p 00000000 00:00 0
         * Thus we update the stack address and size to always have rw permission
         */
        if (!(area.prot & PROT_READ) || !(area.prot & PROT_WRITE)) {
            for (ThreadInfo* thread = thread_list; thread != nullptr; thread = thread->next) {
                void* thread_stack_end_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(thread->stack_addr) + thread->stack_size);
                if ((area.addr >= thread->stack_addr) && (area.addr < thread_stack_end_addr)) {
                    if (area.endAddr >= thread_stack_end_addr)
                        LOG(LL_ERROR, LCF_THREAD, "Top of thread %d stack (%p-%p) detected as not rw because of segment (%p-%p)!",
                            thread->real_tid, thread->stack_addr, thread_stack_end_addr,
                            area.addr, area.endAddr);
                    else {
                        void* new_addr = area.endAddr;
                        size_t new_size = reinterpret_cast<ptrdiff_t>(thread_stack_end_addr) - reinterpret_cast<ptrdiff_t>(area.endAddr);
                        LOG(LL_INFO, LCF_THREAD, "Thread %d stack was updated from %p-%p to %p-%p",
                            thread->real_tid, thread->stack_addr, thread_stack_end_addr,
                            new_addr, thread_stack_end_addr);
                        thread->stack_addr = new_addr;
                        thread->stack_size = new_size;
                    }
                    break;
                }
            }
        }
        
        not_eof = memMapLayout.getNextArea(&area);
    }

    unlockList();
}

}
