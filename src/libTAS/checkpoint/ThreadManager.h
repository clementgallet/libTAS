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

class ThreadManager {
    /* Register start and end time for each routine and each thread. */
    // static std::map<std::ptrdiff_t, std::map<pthread_t, std::vector<TimeHolder>>> startTime_;
    // static std::map<std::ptrdiff_t, std::map<pthread_t, std::vector<TimeHolder>>> endTime_;
    //This will register every pthread created for each start_routine
    // static std::map<std::ptrdiff_t, std::set<pthread_t>> threadMap_;

    static ThreadInfo* thread_list;
    static ThreadInfo* free_list;
    static thread_local ThreadInfo* current_thread;

    static std::map<pthread_t, std::ptrdiff_t> currentAssociation;

    static std::set<std::ptrdiff_t> refTable;
    static std::set<void *> beforeSDL;
    static bool inited;
    static pthread_t main;

    static pthread_mutex_t threadStateLock;
    static pthread_mutex_t threadListLock;
    static pthread_rwlock_t threadResumeLock;
    static sem_t semNotifyCkptThread;
    static sem_t semWaitForCkptThreadSignal;

    static int numThreads;


public:
    static volatile bool restoreInProgress;

    // Called from SDL_init, assumed to be main thread
    static void init();

    /* Create a new ThreadInfo struct from the parent thread*/
    static ThreadInfo* getNewThread();

    /* Init the ThreadInfo with values passed in pthread_create */
    static void initThread(ThreadInfo* thread, pthread_t * tid_p, void * (* start_routine) (void *), void * arg, void * from);

    /* Update the ThreadInfo struct by the child thread */
    static void update(ThreadInfo* thread);

    /* Add a thread to the thread list */
    static void addToList(ThreadInfo* thread);

    /* Remove a thread from the list and add it to the free list */
    static void threadIsDead(ThreadInfo *thread);

    /* Called when thread detach another thread */
    static void threadDetach(pthread_t tid);

    /* Called when thread reaches the end (by return or pthread_exit) */
    static void threadExit();

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


    // Register the start of a new thread
    // It uses the ptrdiff between the start_routine and the calling function
    // ('from'), which is assumed to be a possible invariant
    static void start(pthread_t tid, void *from, void *start_routine);
    // Register the end of a thread
    static void end(pthread_t tid);
    // Should we wait for this entry point ?
    static bool waitFor(pthread_t tid);

    // Attempt to suspend main thread
    static void suspend(pthread_t from_tid);
    // Resumes main thread
    static void resume(pthread_t tid);

    // Display a summary of the current execution (which pthread_t for which entry point)
    // static std::string summary();

    // Sig handler for suspending/resuming thread
    static void sigspin(int sig);
    static std::atomic<int> spin;
};

#endif
