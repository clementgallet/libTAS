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
#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include "ThreadManager.h"
#include "../time.h" // clock_gettime
#include "../threads.h" // getThreadId

std::atomic<int> ThreadManager::spin(0);
ThreadInfo* ThreadManager::thread_list = nullptr;
ThreadInfo* ThreadManager::free_list = nullptr;
thread_local ThreadInfo* ThreadManager::current_thread = nullptr;
std::map<pthread_t, std::ptrdiff_t> ThreadManager::currentAssociation;
std::set<std::ptrdiff_t> ThreadManager::refTable;
std::set<void *> ThreadManager::beforeSDL;
bool ThreadManager::inited = false;
pthread_t ThreadManager::main = 0;

void ThreadManager::sigspin(int sig)
{
    debuglog(LCF_THREAD, "Waiting, sig = ", sig);
    while (spin)
        ;
}

void ThreadManager::init()
{
    // Registering a sighandler enable us to suspend the main thread from any thread !
    struct sigaction sigusr1;
    sigemptyset(&sigusr1.sa_mask);
    sigusr1.sa_flags = 0;
    sigusr1.sa_handler = ThreadManager::sigspin;
    int status = sigaction(SIGUSR1, &sigusr1, nullptr);
    if (status == -1)
        perror("Error installing signal");
    main = getThreadId();
    inited = true;
}

ThreadInfo* ThreadManager::getNewThread()
{
    // TODO: Must lock before accessing free_list.
    ThreadInfo* thread;

    /* Try to recycle a thread from the free list */
    if (free_list) {
        thread = free_list;
        free_list = free_list->next;
        debuglog(LCF_THREAD, "Recycle a ThreadInfo struct");
    }
    else {
        thread = new ThreadInfo;
        debuglog(LCF_THREAD, "Allocate a new ThreadInfo struct");
    }
    return thread;
}

void ThreadManager::initThread(ThreadInfo* thread, pthread_t * tid_p, void * (* start_routine) (void *), void * arg, void * from)
{
    debuglog(LCF_THREAD, "Init thread with routine ", (void*)start_routine);
    thread->tid_p = tid_p;
    thread->tid = 0;
    thread->start = start_routine;
    thread->arg = arg;
    /* 'go' is a small barrier to synchronize the parent thread with the child thread
     * Avoids issues when the created thread is so fast that it's detached
     * before the main thread goes to sleep (ie: starvation => program blocked)
     */
    thread->go = false;
    thread->state = ThreadInfo::ST_RUNNING;
    thread->routine_id = (char *)start_routine - (char *)from;
    thread->next = nullptr;
    thread->prev = nullptr;
}

void ThreadManager::update(ThreadInfo* thread)
{
    debuglog(LCF_THREAD, "Update thread with pthread_t ", stringify(*thread->tid_p));
    thread->tid = *thread->tid_p;
    current_thread = thread;
    addToList(thread);
}

void ThreadManager::addToList(ThreadInfo* thread)
{
    // TODO: Must lock before accessing thread_list.

    /* Check for a thread with the same tid, and remove it */
    ThreadInfo* cur_thread;
    ThreadInfo* next_thread;
    for (cur_thread = thread_list; cur_thread != nullptr; cur_thread = next_thread) {
        next_thread = cur_thread->next;

        if (cur_thread->tid == thread->tid) {
            threadIsDead(cur_thread);
            continue;
        }
    }

    /* Add the new thread to the list */
    thread->next = thread_list;
    thread->prev = nullptr;
    if (thread_list != nullptr) {
        thread_list->prev = thread;
    }
    thread_list = thread;
}

void ThreadManager::threadIsDead(ThreadInfo *thread)
{
    debuglog(LCF_THREAD, "Remove thread ", stringify(thread->tid), " from list");

    if (thread->prev != nullptr) {
        thread->prev->next = thread->next;
    }
    if (thread->next != nullptr) {
        thread->next->prev = thread->prev;
    }
    if (thread == thread_list) {
        thread_list = thread_list->next;
    }

    thread->next = free_list;
    free_list = thread;
}

void ThreadManager::threadExit()
{
    current_thread->state = ThreadInfo::ST_ZOMBIE;
}

void ThreadManager::deallocateThreads()
{
    // TODO: Must lock before accessing thread_list.
    while (free_list != nullptr) {
        ThreadInfo *thread = free_list;
        free_list = free_list->next;
        free(thread);
    }
}






void ThreadManager::suspend(pthread_t from_tid)
{
    // We want to suspend main if:
    //  - from_tid is main (which means it asks for it)
    //  - from_tid is one of the registered threads we want to wait for
    if (from_tid == main || waitFor(from_tid)) {
        debuglog(LCF_THREAD, "Suspending main (", stringify(main), ") because of ", stringify(from_tid));
        spin = 1;
        // This doesn't actually kill the thread, it send SIGUSR1 to the main
        // thread, which make it spins until resume
        pthread_kill(main, SIGUSR1);
    } else {
        debuglog(LCF_THREAD, "Not suspending because of ", stringify(from_tid));
    }
}

void ThreadManager::start(pthread_t tid, void *from, void *start_routine)
{
    if (!inited) {
        beforeSDL.insert(start_routine);
        return;
    }
    ptrdiff_t diff = (char *)start_routine - (char *)from;
    // std::set<pthread_t> &all = threadMap_[diff];
    // all.insert(tid);
    // Register the current thread id -> routine association
    // The same tid can be reused with a different routine, but different routines
    // cannot be running at the same time with the same tid.
    currentAssociation[tid] = diff;
    debuglog(LCF_THREAD, "Register starting ", stringify(tid)," with entrydiff ",  diff, ".");
    // TimeHolder t;
    // {
    //     GlobalNative tn;
    //     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    // }
    //There may be multiple call to start...

    // startTime_[diff][tid].push_back(t);
}

void ThreadManager::end(pthread_t tid)
{
    debuglog(LCF_THREAD, "Register ending ", stringify(tid), ".");
    // TimeHolder t;
    // {
    //     GlobalNative tn;
    //     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    // }
    // ptrdiff_t diff = currentAssociation[tid];
    // endTime_[diff][tid].push_back(t);
}

void ThreadManager::resume(pthread_t tid)
{
    if (!waitFor(tid))
        return;
    if (spin) {
        spin = 0;
        debuglog(LCF_THREAD, "Released main.");
    }
}

bool ThreadManager::waitFor(pthread_t tid)
{
    // Lookup the entry point corresponding to this tid, and check if it's
    // in the reference table
    ptrdiff_t diff = currentAssociation[tid];
    return inited && refTable.count(diff);
}

// std::string ThreadManager::summary()
// {
//     std::ostringstream oss;
//     for (auto elem : threadMap_) {
//         oss << "\nRecord for entry point : " << elem.first;
//         std::set<pthread_t> &allThread = elem.second;
//         for (pthread_t t : allThread) {
//             oss << "\n  - " << stringify(t);
//             //FIXME using find would permit to add the const qualifier to this member
//             std::vector<TimeHolder> &starts = startTime_[elem.first][t];
//             if (starts.empty())
//                 continue;
//             std::vector<TimeHolder> &ends = endTime_[elem.first][t];
//             for (unsigned int i = 0; i < starts.size(); i++) {
//                 oss << "\n    1: Started";
//                 TimeHolder start = starts[i];
//                 if (i < ends.size()) {
//                     TimeHolder end = ends[i];
//                     TimeHolder diff = end - start;
//                     oss << " and lasted " << diff.tv_sec << " seconds and " << diff.tv_nsec << " nsec.";
//                 }
//             }
//         }
//     }
//     oss << "\nThese threads started before SDL init and can't be waited for :\n";
//     for (auto elem : beforeSDL_) {
//         oss <<  elem << "\n";
//     }
//     return oss.str();
// }
