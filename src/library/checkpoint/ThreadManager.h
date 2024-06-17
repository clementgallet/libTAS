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

    Author : Philippe Virouleau <philippe.virouleau@imag.fr>
 */

#ifndef LIBTAS_THREAD_MANAGER_H
#define LIBTAS_THREAD_MANAGER_H

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
    
namespace ThreadManager {

// Called from SDL_init, assumed to be main thread
void init();

/* Get the pthread id */
pthread_t getThreadId();

/* Get the thread tid */
pid_t getThreadTid();

/* Restore the true tid of the current thread, and set it into the internal pthread structure */
void restoreTid();

/* Set the main thread to this thread */
void setMainThread();

/* Set the main thread to the provided thread */
void setMainThread(pthread_t pthread_id);

/* Check if this thread is main thread */
bool isMainThread();

/* Set the checkpoint thread to this thread */
void setCheckpointThread();

/* Get the thread tid of another thread */
pid_t getThreadTid(pthread_t pthread_id);

/* Get the ThreadInfo struct from the thread id, or null if not there */
ThreadInfo* getThread(pthread_t pthread_id);

/* Init the ThreadInfo by the parent thread with values passed in pthread_create */
void initThreadFromParent(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from);

/* Get offset of tid inside pthread_t struct */
int getTidOffset();

/* Finish the initialization of the ThreadInfo struct by the child thread */
void initThreadFromChild(ThreadInfo* thread);

/* Set the global state based on thread state */
void setGlobalState(ThreadInfo* thread);

/* Add a thread to the thread list */
void addToList(ThreadInfo* thread);

/* Get the thread list */
ThreadInfo* getThreadList();

/* Are we a child process for state saving? */
bool isChildFork();

/* Set the child process state */
void setChildFork();

/* Remove a thread from the list and add it to the free list */
void threadIsDead(ThreadInfo *thread);

/* Called when thread detach another thread */
void threadDetach(pthread_t pthread_id);

/* Called when thread reaches the end (by return or pthread_exit).
 * Store the returned value.
 */
void threadExit(void* retval);

/* Deallocate all ThreadInfo structs from the free list */
void deallocateThreads();

/* Safely try to change a ThreadInfo state and return if done */
bool updateState(ThreadInfo *th, ThreadInfo::ThreadState newval, ThreadInfo::ThreadState oldval);

/* Get the current thread */
ThreadInfo *getCurrentThread();

/* A function called from pthread_start() to fix current_thread
 * after pthread_start() erases all thread_local variables.
 */
void setCurrentThread(ThreadInfo *thread);

/* Lock or unlock the mutex when modifying the thread list */
void lockList();
void unlockList();

/* Update the thread stack address and size based on what actual memory is
 * available */
void updateStackInfo();

}
}

#endif
