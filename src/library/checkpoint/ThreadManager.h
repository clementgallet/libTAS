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

    Author : Philippe Virouleau <philippe.virouleau@imag.fr>
 */

#ifndef LIBTAS_THREAD_MANAGER_H
#define LIBTAS_THREAD_MANAGER_H
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
class ThreadManager {
    static ThreadInfo* free_list;
    static thread_local ThreadInfo* current_thread;

    // static bool inited;
    static pthread_t main_pthread_id;

    static pthread_mutex_t threadStateLock;
    static pthread_mutex_t threadListLock;
    static pthread_rwlock_t threadResumeLock;
    static sem_t semNotifyCkptThread;
    static sem_t semWaitForCkptThreadSignal;

    static int numThreads;

public:
    static ThreadInfo* thread_list;

    static volatile bool restoreInProgress;

    // Called from SDL_init, assumed to be main thread
    static void init();

    /* Get the pthread id */
    static pthread_t getThreadId();

    /* Get the thread tid */
    static pid_t getThreadTid();

    /* Set the main thread to this thread */
    static void setMainThread();

    /* Check if this thread is main thread */
    static bool isMainThread();

    /* Create a new ThreadInfo struct from the parent thread*/
    static ThreadInfo* getNewThread();

    /* Get the thread tid of another thread */
    static pid_t getThreadTid(pthread_t pthread_id);

    /* Get the ThreadInfo struct from the thread id, or null if not there */
    static ThreadInfo* getThread(pthread_t pthread_id);

    /* Init the ThreadInfo with values passed in pthread_create, and return if
     * the thread was recycled or not.
     */
    static bool initThread(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from);

    /* Update the ThreadInfo struct by the child thread */
    static void update(ThreadInfo* thread);

    /* Add a thread to the thread list */
    static void addToList(ThreadInfo* thread);

    /* Remove a thread from the list and add it to the free list */
    static void threadIsDead(ThreadInfo *thread);

    /* Called when thread detach another thread */
    static void threadDetach(pthread_t pthread_id);

    /* Called when thread reaches the end (by return or pthread_exit).
     * Store the returned value.
     */
    static void threadExit(void* retval);

    /* Deallocate all ThreadInfo structs from the free list */
    static void deallocateThreads();

    /* Checkpoint */
    static void checkpoint();

    /* Restore */
    static void restore();

    /* Safely try to change a ThreadInfo state and return if done */
    static bool updateState(ThreadInfo *th, ThreadInfo::ThreadState newval, ThreadInfo::ThreadState oldval);

    /* Send a signal to suspend all threads before checkpointing */
    static void suspendThreads();

    /* Resume all threads */
    static void resumeThreads();

    /* Function executed by all secondary threads using signal SIGUSR1 */
    static void stopThisThread(int signum);

    static void waitForAllRestored(ThreadInfo *thread);

    /* A function called from pthread_start() to fix current_thread
     * after pthread_start() erases all thread_local variables.
     */
    static void setCurrentThread(ThreadInfo *thread) {
      current_thread = thread;
    }

};
}

#endif
