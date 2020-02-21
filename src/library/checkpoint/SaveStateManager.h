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

#ifndef LIBTAS_SAVESTATE_MANAGER_H
#define LIBTAS_SAVESTATE_MANAGER_H
#include "../TimeHolder.h"
#include "ThreadInfo.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <semaphore.h>

namespace libtas {
namespace SaveStateManager {

// static thread_local ThreadInfo* current_thread;
//
// static pthread_mutex_t threadStateLock;
// static pthread_mutex_t threadListLock;
// static pthread_rwlock_t threadResumeLock;
// static sem_t semNotifyCkptThread;
// static sem_t semWaitForCkptThreadSignal;
//
// static int numThreads;
//
// /* Which signals are we using */
// int sig_suspend_threads;
// int sig_checkpoint;
//
// volatile bool restoreInProgress;

void init();

/* Initialize the signal handler for the checkpoint thread */
void initCheckpointThread();

void initThreadFromChild(ThreadInfo* thread);

/* Save a savestate and returns if succeeded */
bool checkpoint();

/* Restore a savestate */
void restore();

/* Send a signal to suspend all threads before checkpointing */
void suspendThreads();

/* Resume all threads */
void resumeThreads();

/* Function executed by all secondary threads using signal SIGUSR1 */
void stopThisThread(int signum);

void waitForAllRestored(ThreadInfo *thread);

/* Is currently loading a savestate? */
bool isLoading();

/* Restore the state of loading a savestate, after memory has been rewritten */
void setLoading();

/* Returns the signal number of checkpoint and thread suspend */
int sigCheckpoint();
int sigSuspend();

}
}

#endif
