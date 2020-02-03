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
#include "../GlobalState.h"
#include <atomic>
#include <pthread.h> // pthread_rwlock_t

namespace libtas {

static std::atomic<int> uninitializedThreadCount(0);
static pthread_rwlock_t wrapperExecutionLock =
    PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP;

void ThreadSync::acquireLocks()
{
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Waiting for other threads to exit wrappers");
    MYASSERT(pthread_rwlock_wrlock(&wrapperExecutionLock) == 0)

    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Waiting for newly created threads to finish initialization");
    waitForThreadsToFinishInitialization();

    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Done acquiring all locks");
}

void ThreadSync::releaseLocks()
{
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Releasing ThreadSync locks");
    MYASSERT(pthread_rwlock_unlock(&wrapperExecutionLock) == 0)
}

void ThreadSync::waitForThreadsToFinishInitialization()
{
    while (uninitializedThreadCount != 0) {
        struct timespec sleepTime = { 0, 10 * 1000 * 1000 };
        debuglog(LCF_THREAD, "Sleeping ", sleepTime.tv_nsec, " ns for thread initialization");
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
        debuglog(LCF_ERROR | LCF_THREAD, "uninitializedThreadCount is negative!");
    }
    uninitializedThreadCount--;
}

void ThreadSync::wrapperExecutionLockLock()
{
    while (1) {
        int retVal = pthread_rwlock_tryrdlock(&wrapperExecutionLock);
        if (retVal != 0 && retVal == EBUSY) {
            struct timespec sleepTime = { 0, 100 * 1000 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
            continue;
        }
        if (retVal != 0 && retVal != EDEADLK) {
            debuglog(LCF_ERROR | LCF_THREAD, "Failed to acquire lock!");
            return;
        }
        break;
    }
}

void ThreadSync::wrapperExecutionLockUnlock()
{
    if (pthread_rwlock_unlock(&wrapperExecutionLock) != 0) {
        debuglog(LCF_ERROR | LCF_THREAD, "Failed to release lock!");
    }
}

}
