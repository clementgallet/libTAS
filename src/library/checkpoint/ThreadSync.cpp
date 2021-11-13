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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */

#include "ThreadSync.h"
#include "../logging.h"
#include <time.h> // nanosleep
#include "../sleepwrappers.h"
#include "../GlobalState.h"
#include <atomic>
#include <pthread.h> // pthread_rwlock_t
#include <mutex>
#include <condition_variable>

namespace libtas {

static std::atomic<int> uninitializedThreadCount(0);
static pthread_mutex_t wrapperExecutionLock = PTHREAD_MUTEX_INITIALIZER;
static std::mutex detMutex;
static std::condition_variable detCond;
static bool syncGo[10];


void ThreadSync::acquireLocks()
{
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Waiting for other threads to exit wrappers");
    MYASSERT(pthread_mutex_lock(&wrapperExecutionLock) == 0)

    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Waiting for newly created threads to finish initialization");
    waitForThreadsToFinishInitialization();

    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Done acquiring all locks");
}

void ThreadSync::releaseLocks()
{
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Releasing ThreadSync locks");
    MYASSERT(pthread_mutex_unlock(&wrapperExecutionLock) == 0)
}

void ThreadSync::waitForThreadsToFinishInitialization()
{
    while (uninitializedThreadCount != 0) {
        struct timespec sleepTime = { 0, 10 * 1000 * 1000 };
        debuglogstdio(LCF_THREAD, "Sleeping %d ns for thread initialization", sleepTime.tv_nsec);
        NATIVECALL(nanosleep(&sleepTime, NULL));
    }
}

void ThreadSync::incrementUninitializedThreadCount()
{
    uninitializedThreadCount++;
}

void ThreadSync::decrementUninitializedThreadCount()
{
    if (uninitializedThreadCount <= 0) {
        debuglogstdio(LCF_ERROR | LCF_THREAD, "uninitializedThreadCount is negative!");
    }
    uninitializedThreadCount--;
}

void ThreadSync::wrapperExecutionLockLock()
{
    while (1) {
        int retVal = pthread_mutex_trylock(&wrapperExecutionLock);
        if (retVal != 0 && retVal == EBUSY) {
            struct timespec sleepTime = { 0, 100 * 1000 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
            continue;
        }
        if (retVal != 0 && retVal != EDEADLK) {
            debuglogstdio(LCF_ERROR | LCF_THREAD, "Failed to acquire lock!");
            return;
        }
        break;
    }
}

void ThreadSync::wrapperExecutionLockUnlock()
{
    if (pthread_mutex_unlock(&wrapperExecutionLock) != 0) {
        debuglogstdio(LCF_ERROR | LCF_THREAD, "Failed to release lock!");
    }
}

void ThreadSync::detInit()
{
    ThreadInfo *current_thread = ThreadManager::getCurrentThread();
    current_thread->syncEnabled = true;
    current_thread->syncGo = false;
}

void ThreadSync::detWait()
{
    bool shouldWait = true;

    while (shouldWait) {
        shouldWait = false;
        for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
            /* Should wait if sync count has increased since last time */
            if (thread->syncCount > thread->syncOldCount) {
                // debuglogstdio(LCF_ERROR, "Thread %d has increased sync count %d -> %d", thread->tid, thread->syncOldCount, thread->syncCount.load());
                thread->syncOldCount = thread->syncCount;
                shouldWait = true;
            }

            if (!thread->syncEnabled) continue;
            if (!thread->syncGo) {
                shouldWait = true;
                std::unique_lock<std::mutex> lock(detMutex);
                detCond.wait(lock, [thread]{ return (thread->syncGo); });
                thread->syncGo = false;
            }
        }
        if (shouldWait)
            NATIVECALL(usleep(100));
    }

    /* Reset sync count */
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        thread->syncCount = 0;
        thread->syncOldCount = 0;
    }
}

void ThreadSync::detWaitGlobal(int i)
{
    debuglogstdio(LCF_THREAD, "Wait on global lock %d", i);
    std::unique_lock<std::mutex> lock(detMutex);
    detCond.wait(lock, [i]{ return (syncGo[i]); });
    syncGo[i] = false;
    debuglogstdio(LCF_THREAD, "End Wait on global lock %d", i);
}

void ThreadSync::detSignal(bool stop)
{
    ThreadInfo *current_thread = ThreadManager::getCurrentThread();

    if (!current_thread->syncEnabled)
        return;
    {
        std::lock_guard<std::mutex> lock(detMutex);
        current_thread->syncGo = true;
        current_thread->syncCount++;
    }
    detCond.notify_all();

    if (stop)
        current_thread->syncEnabled = false;
}

void ThreadSync::detSignalGlobal(int i)
{
    debuglogstdio(LCF_THREAD, "Signal global lock %d", i);
    {
        std::lock_guard<std::mutex> lock(detMutex);
        syncGo[i] = true;
    }
    detCond.notify_all();
}

}
