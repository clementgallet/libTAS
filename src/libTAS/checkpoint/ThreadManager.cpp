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
    Part of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */
#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include "ThreadManager.h"
#include "ThreadSync.h"
#include "Checkpoint.h"
#include "../timewrappers.h" // clock_gettime
#include "../threadwrappers.h" // getThreadId
#include "../logging.h"
// #include "../backtrace.h"
#include "../audio/AudioPlayer.h"

namespace libtas {

std::atomic<int> ThreadManager::spin(0);
ThreadInfo* ThreadManager::thread_list = nullptr;
ThreadInfo* ThreadManager::free_list = nullptr;
thread_local ThreadInfo* ThreadManager::current_thread = nullptr;
std::map<pthread_t, std::ptrdiff_t> ThreadManager::currentAssociation;
std::set<std::ptrdiff_t> ThreadManager::refTable;
std::set<void *> ThreadManager::beforeSDL;
bool ThreadManager::inited = false;
pthread_t ThreadManager::main = 0;
pthread_mutex_t ThreadManager::threadStateLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadManager::threadListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t ThreadManager::threadResumeLock = PTHREAD_RWLOCK_INITIALIZER;
sem_t ThreadManager::semNotifyCkptThread;
sem_t ThreadManager::semWaitForCkptThreadSignal;
volatile bool ThreadManager::restoreInProgress = false;
int ThreadManager::numThreads;

void ThreadManager::sigspin(int sig)
{
    debuglog(LCF_THREAD, "Waiting, sig = ", sig);
    while (spin)
        ;
}

void ThreadManager::init()
{
    /* Create a ThreadInfo struct for this thread */
    ThreadInfo* thread = ThreadManager::getNewThread();
    thread->state = ThreadInfo::ST_CKPNTHREAD;
    thread->tid = getThreadId();
    thread->detached = false;
    current_thread = thread;
    addToList(thread);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    // NATIVECALL(sigprocmask(SIG_UNBLOCK, &mask, nullptr));
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));

    sem_init(&semNotifyCkptThread, 0, 0);
    sem_init(&semWaitForCkptThreadSignal, 0, 0);

    // Registering a sighandler enable us to suspend the main thread from any thread !
    // struct sigaction sigusr1;
    // sigemptyset(&sigusr1.sa_mask);
    // sigusr1.sa_flags = 0;
    // sigusr1.sa_handler = ThreadManager::sigspin;
    // int status = sigaction(SIGUSR1, &sigusr1, nullptr);
    // if (status == -1)
    //     perror("Error installing signal");

    struct sigaction sigusr1;
    sigfillset(&sigusr1.sa_mask);
    sigusr1.sa_flags = SA_RESTART;
    sigusr1.sa_handler = ThreadManager::stopThisThread;
    {
        GlobalOwnCode goc;
        MYASSERT(sigaction(SIGUSR1, &sigusr1, nullptr) == 0)
    }
    Checkpoint::init();

    main = getThreadId();
    inited = true;
}

ThreadInfo* ThreadManager::getNewThread()
{
    ThreadInfo* thread;

    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
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

    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
    return thread;
}

ThreadInfo* ThreadManager::getThread(pthread_t tid)
{
    for (ThreadInfo* thread = thread_list; thread != nullptr; thread = thread->next)
        if (thread->tid == tid)
            return thread;

    return nullptr;
}

void ThreadManager::initThread(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from)
{
    debuglog(LCF_THREAD, "Init thread with routine ", (void*)start_routine);
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
    thread->detached = false;
    thread->next = nullptr;
    thread->prev = nullptr;
}

void ThreadManager::update(ThreadInfo* thread)
{
    thread->tid = getThreadId();
    current_thread = thread;
    addToList(thread);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));
}

void ThreadManager::addToList(ThreadInfo* thread)
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)

    /* Check for a thread with the same tid, and remove it */
    ThreadInfo* cur_thread = getThread(thread->tid);
    if (cur_thread)
        threadIsDead(cur_thread);

    /* Add the new thread to the list */
    thread->next = thread_list;
    thread->prev = nullptr;
    if (thread_list != nullptr) {
        thread_list->prev = thread;
    }
    thread_list = thread;

    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
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

void ThreadManager::threadDetach(pthread_t tid)
{
    ThreadInfo* thread = getThread(tid);

    if (thread) {
        MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
        thread->detached = true;
        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            debuglog(LCF_THREAD, "Zombie thread ", stringify(tid), " is detached");
            threadIsDead(thread);
        }
        MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
    }
}

void ThreadManager::threadExit()
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
    MYASSERT(updateState(current_thread, ThreadInfo::ST_ZOMBIE, ThreadInfo::ST_RUNNING))
    if (current_thread->detached) {
        debuglog(LCF_THREAD, "Detached thread ", stringify(current_thread->tid), " exited");
        threadIsDead(current_thread);
    }
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::deallocateThreads()
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
    while (free_list != nullptr) {
        ThreadInfo *thread = free_list;
        free_list = free_list->next;
        free(thread);
    }
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::checkpoint(const char* savestatepath)
{
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)
    // debuglog(LCF_THREAD | LCF_CHECKPOINT, "Checkpo fi ", savestatepath);

    ThreadSync::acquireLocks();

    Checkpoint::setSavestatePath(savestatepath);

    restoreInProgress = false;

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
    AudioPlayer::close();

    suspendThreads();

    /* All other threads halted in 'stopThisThread' routine (they are all
     * in state ST_SUSPENDED).  It's safe to write checkpoint file now.
     * We call the write function with a signal, so we can use an alternate stack
     * and safely writing to the original stack.
     */

    raise(SIGUSR2);

    resumeThreads();

    if (restoreInProgress) {
        /* We are being restored.  Wait for all other threads to finish being
         * restored before resuming.
         */
        debuglog(LCF_THREAD | LCF_CHECKPOINT, "Waiting for other threads after restore");
        waitForAllRestored(current_thread);
        debuglog(LCF_THREAD | LCF_CHECKPOINT, "Resuming after restore");
    }

    ThreadSync::releaseLocks();
}

void ThreadManager::restore(const char* savestatepath)
{
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)
    ThreadSync::acquireLocks();
    // debuglog(LCF_THREAD | LCF_CHECKPOINT, "Restore fi ", savestatepath);

    Checkpoint::setSavestatePath(savestatepath);

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
    AudioPlayer::close();

    /* Perform a series of checks before attempting to restore */
    if (!Checkpoint::checkRestore()) {
        ThreadSync::releaseLocks();
        return;
    }

    restoreInProgress = false;

    suspendThreads();

    restoreInProgress = true;

    /* Here is where we load all the memory and stuff */
    raise(SIGUSR2);

    /* It seems that when restoring the original stack at the end of the
     * signal handler function, the program pulls from the stack the address
     * to return to. Because we replace the stack memory with the checkpointed
     * stack, we will return to after `raise(SIGUSR2)` call in
     * ThreadManager::checkpoint(). We may even not have to use
     * getcontext/setcontext for this thread!
     */

     /* If restore was not done, we return here */
     debuglog(LCF_THREAD | LCF_CHECKPOINT, "Restoring was not done, resuming threads");
     resumeThreads();
     waitForAllRestored(current_thread);

     ThreadSync::releaseLocks();
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

void ThreadManager::suspendThreads()
{
    MYASSERT(pthread_rwlock_destroy(&threadResumeLock) == 0)
    MYASSERT(pthread_rwlock_init(&threadResumeLock, NULL) == 0)
    MYASSERT(pthread_rwlock_wrlock(&threadResumeLock) == 0)

    /* Halt all other threads - force them to call stopthisthread
    * If any have blocked checkpointing, wait for them to unblock before
    * signalling
    */
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)

    bool needrescan = false;
    do {
        needrescan = false;
        numThreads = 0;
        ThreadInfo *next;
        for (ThreadInfo *thread = thread_list; thread != nullptr; thread = next) {
            debuglog(LCF_THREAD | LCF_CHECKPOINT, "Signaling thread ", stringify(thread->tid));
            next = thread->next;
            int ret;

            /* Do various things based on thread's state */
            switch (thread->state) {
            case ThreadInfo::ST_RUNNING:

                /* Thread is running. Send it a signal so it will call stopthisthread.
                * We will need to rescan (hopefully it will be suspended by then)
                */
                if (updateState(thread, ThreadInfo::ST_SIGNALED, ThreadInfo::ST_RUNNING)) {
                    ret = pthread_kill(thread->tid, SIGUSR1);

                    if (ret == 0) {
                        needrescan = true;
                    }
                    else {
                        MYASSERT(ret == ESRCH)
                        debuglog(LCF_THREAD | LCF_CHECKPOINT, "Thread", stringify(thread->tid), "has died since");
                        threadIsDead(thread);
                    }
                }
                break;

            case ThreadInfo::ST_ZOMBIE:

                /* Zombie threads are detached here, to avoid getting leaking
                 * threads after state loading or other kinds of errors.
                 * We have to get the return value if the game wants it later
                 */

                debuglog(LCF_THREAD | LCF_CHECKPOINT, "Zombie thread ", stringify(thread->tid), " is joined");
                updateState(thread, ThreadInfo::ST_FAKEZOMBIE, ThreadInfo::ST_ZOMBIE);
                NATIVECALL(pthread_join(thread->tid, &thread->retval));
                break;

            case ThreadInfo::ST_SIGNALED:
                ret = pthread_kill(thread->tid, 0);

                if (ret == 0) {
                    needrescan = true;
                }
                else {
                    MYASSERT(ret == ESRCH)
                    debuglog(LCF_ERROR | LCF_THREAD | LCF_CHECKPOINT, "Signalled thread ", stringify(thread->tid), " died");
                    threadIsDead(thread);
                }
                break;

            case ThreadInfo::ST_SUSPINPROG:
                numThreads++;
                break;

            case ThreadInfo::ST_SUSPENDED:
                numThreads++;
                break;

            case ThreadInfo::ST_CKPNTHREAD:
                break;

            case ThreadInfo::ST_FAKEZOMBIE:
                break;

            default:
                debuglog(LCF_ERROR | LCF_THREAD | LCF_CHECKPOINT, "Unknown thread state ", thread->state);
            }
        }
        if (needrescan) {
            struct timespec sleepTime = { 0, 10 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
        }
    } while (needrescan);

    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)

    for (int i = 0; i < numThreads; i++) {
        sem_wait(&semNotifyCkptThread);
    }

    debuglog(LCF_THREAD | LCF_CHECKPOINT, numThreads, " threads were suspended");
}

/* Resume all threads. */
void ThreadManager::resumeThreads()
{
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Resuming all threads");
    MYASSERT(pthread_rwlock_unlock(&threadResumeLock) == 0)
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "All threads resumed");
}

void ThreadManager::stopThisThread(int signum)
{
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Received suspend signal!");
    if (current_thread->state == ThreadInfo::ST_CKPNTHREAD) {
        return;
    }

    /* Make sure we don't get called twice for same thread */
    if (updateState(current_thread, ThreadInfo::ST_SUSPINPROG, ThreadInfo::ST_SIGNALED)) {

        /* Set up our restart point, ie, we get jumped to here after a restore */

        // TLSInfo_SaveTLSState(&curThread->tlsInfo); // save thread local storage
        MYASSERT(getcontext(&current_thread->savctx) == 0)
        // save_sp(&curThread->saved_sp);

        debuglog(LCF_THREAD | LCF_CHECKPOINT, "Thread after getcontext");
        // printBacktrace();

        if (!restoreInProgress) {

            /* We are a user thread and all context is saved.
             * Wait for ckpt thread to write ckpt, and resume.
             */

            /* Tell the checkpoint thread that we're all saved away */
            MYASSERT(updateState(current_thread, ThreadInfo::ST_SUSPENDED, ThreadInfo::ST_SUSPINPROG))
            sem_post(&semNotifyCkptThread);

            /* Then wait for the ckpt thread to write the ckpt file then wake us up */
            debuglog(LCF_THREAD | LCF_CHECKPOINT, "Thread suspended");

            MYASSERT(pthread_rwlock_rdlock(&threadResumeLock) == 0)
            MYASSERT(pthread_rwlock_unlock(&threadResumeLock) == 0)

            /* If when thread was suspended, we performed a restore,
             * then we must resume execution using setcontext
             */
            if (restoreInProgress) {
                setcontext(&current_thread->savctx);
                /* NOT REACHED */
            }
            MYASSERT(updateState(current_thread, ThreadInfo::ST_RUNNING, ThreadInfo::ST_SUSPENDED))
        }
        else {
            /* We successfully restored the thread. We wait for all other
             * threads to restore before continuing
             */
            MYASSERT(updateState(current_thread, ThreadInfo::ST_RUNNING, ThreadInfo::ST_SUSPENDED))

            /* Else restoreinprog >= 1;  This stuff executes to do a restart */
            waitForAllRestored(current_thread);
        }

    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Thread returning to user code");
    }
}

void ThreadManager::waitForAllRestored(ThreadInfo *thread)
{
    if (thread->state == ThreadInfo::ST_CKPNTHREAD) {
        for (int i = 0; i < numThreads; i++) {
            sem_wait(&semNotifyCkptThread);
        }

        /* If this was last of all, wake everyone up */
        for (int i = 0; i < numThreads; i++) {
            sem_post(&semWaitForCkptThreadSignal);
        }
    }
    else {
        sem_post(&semNotifyCkptThread);
        sem_wait(&semWaitForCkptThreadSignal);
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

}
