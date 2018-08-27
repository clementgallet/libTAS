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
    Part of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */
#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include <sys/syscall.h> // syscall, SYS_gettid

#include "ThreadManager.h"
#include "ThreadSync.h"
#include "Checkpoint.h"
#include "../timewrappers.h" // clock_gettime
#include "../threadwrappers.h" // getThreadId
#include "../logging.h"
// #include "../backtrace.h"
#include "../audio/AudioPlayer.h"
#include "AltStack.h"
#include "CustomSignals.h"
#include "ReservedMemory.h"

namespace libtas {

ThreadInfo* ThreadManager::thread_list = nullptr;
thread_local ThreadInfo* ThreadManager::current_thread = nullptr;
pthread_t ThreadManager::main_pthread_id = 0;
pthread_mutex_t ThreadManager::threadStateLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadManager::threadListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t ThreadManager::threadResumeLock = PTHREAD_RWLOCK_INITIALIZER;
sem_t ThreadManager::semNotifyCkptThread;
sem_t ThreadManager::semWaitForCkptThreadSignal;
volatile bool ThreadManager::restoreInProgress = false;
int ThreadManager::numThreads;

void ThreadManager::init()
{
    /* Create a ThreadInfo struct for this thread */
    ThreadInfo* thread = ThreadManager::getNewThread();
    thread->state = ThreadInfo::ST_RUNNING;
    thread->pthread_id = getThreadId();
    thread->tid = syscall(SYS_gettid);
    thread->detached = false;
    current_thread = thread;
    addToList(thread);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));

    sem_init(&semNotifyCkptThread, 0, 0);
    sem_init(&semWaitForCkptThreadSignal, 0, 0);

    ReservedMemory::init();

    setMainThread();
}

DEFINE_ORIG_POINTER(pthread_self);

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
    if (current_thread)
        return current_thread->tid;
    return 0;
}

void ThreadManager::setMainThread()
{
    main_pthread_id = getThreadId();
    current_thread->state = ThreadInfo::ST_CKPNTHREAD;
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

    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
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
        memset(thread, 0, sizeof(ThreadInfo));
        debuglog(LCF_THREAD, "Allocate a new ThreadInfo struct");
    }

    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
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

bool ThreadManager::initThread(ThreadInfo* thread, void * (* start_routine) (void *), void * arg, void * from)
{
    debuglog(LCF_THREAD, "Init thread with routine ", (void*)start_routine);
    bool isRecycled = thread->state == ThreadInfo::ST_RECYCLED;

    thread->start = start_routine;
    thread->arg = arg;
    thread->state = ThreadInfo::ST_RUNNING;
    thread->routine_id = (char *)start_routine - (char *)from;
    thread->detached = false;
    thread->initial_native = GlobalState::isNative();
    thread->initial_owncode = GlobalState::isOwnCode();
    thread->initial_nolog = GlobalState::isNoLog();

    if (!isRecycled) {
        thread->pthread_id = 0;
        thread->tid = 0;

        if (!thread->altstack.ss_sp) {
            thread->altstack.ss_size = 64*1024;
            thread->altstack.ss_sp = malloc(thread->altstack.ss_size);
            thread->altstack.ss_flags = 0;
        }

        thread->next = nullptr;
        thread->prev = nullptr;
    }

    return isRecycled;
}

void ThreadManager::update(ThreadInfo* thread)
{
    thread->pthread_id = getThreadId();
    thread->tid = syscall(SYS_gettid);

    /* If a thread that is in native state is creating another thread, we
     * consider that the entire new thread is in native mode (e.g. audio thread)
     */
    if (thread->initial_native) GlobalState::setNative(true);
    if (thread->initial_owncode) GlobalState::setOwnCode(true);
    if (thread->initial_nolog) GlobalState::setNoLog(true);

    current_thread = thread;
    addToList(thread);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));

    /* The signal handler for the thread suspend must be registered on *each*
     * thread, because it runs on a custom stack.
     */
    int ret;
    NATIVECALL(ret = sigaltstack(&thread->altstack, nullptr));
    if (ret < 0) {
        debuglog(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "sigaltstack failed with error ", errno);
        debuglog(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "stack starts at ", thread->altstack.ss_sp);
    }

    struct sigaction sigusr1;
    sigfillset(&sigusr1.sa_mask);
    sigusr1.sa_flags = SA_RESTART | SA_ONSTACK;
    sigusr1.sa_handler = stopThisThread;
    {
        GlobalNative gn;
        MYASSERT(sigaction(SIGUSR1, &sigusr1, nullptr) == 0)
    }
}

void ThreadManager::addToList(ThreadInfo* thread)
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)

    /* Check for a thread with the same tid. If it is the same thread then we
     * have nothing to do. Otherwise, remove it.
     */
    ThreadInfo* cur_thread = getThread(thread->pthread_id);
    if (cur_thread) {
        if (thread == cur_thread) {
            MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
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

    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::threadIsDead(ThreadInfo *thread)
{
    debuglog(LCF_THREAD, "Remove thread ", thread->tid, " from list");

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
}

void ThreadManager::threadDetach(pthread_t pthread_id)
{
    ThreadInfo* thread = getThread(pthread_id);

    if (thread) {
        MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
        thread->detached = true;
        if (thread->state == ThreadInfo::ST_ZOMBIE) {
            debuglog(LCF_THREAD, "Zombie thread ", thread->tid, " is detached");
            MYASSERT(updateState(thread, ThreadInfo::ST_FREE, ThreadInfo::ST_ZOMBIE))
            if (! shared_config.recycle_threads) {
                threadIsDead(thread);
            }
        }
        MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
    }
}

void ThreadManager::threadExit(void* retval)
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)
    current_thread->retval = retval;
    MYASSERT(updateState(current_thread, ThreadInfo::ST_ZOMBIE, ThreadInfo::ST_RUNNING))
    if (current_thread->detached) {
        debuglog(LCF_THREAD, "Detached thread ", current_thread->tid, " exited");
        MYASSERT(updateState(current_thread, ThreadInfo::ST_FREE, ThreadInfo::ST_ZOMBIE))
        if (! shared_config.recycle_threads) {
            threadIsDead(current_thread);
        }
    }
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::deallocateThreads()
{
    MYASSERT(pthread_mutex_lock(&threadListLock) == 0)

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
    MYASSERT(pthread_mutex_unlock(&threadListLock) == 0)
}

void ThreadManager::checkpoint()
{
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)

    ThreadSync::acquireLocks();

    restoreInProgress = false;

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
    AudioPlayer::close();

    /* Perform a series of checks before attempting to checkpoint */
    if (!Checkpoint::checkCheckpoint()) {
        ThreadSync::releaseLocks();
        return;
    }

    /* Before suspending threads, we must also register our signal handlers */
    CustomSignals::registerHandlers();

    /* We save the alternate stack if the game did set one */
    AltStack::saveStack();

    /* Sending a suspend signal to all threads */
    suspendThreads();

    /* We set the alternate stack to our reserved memory. The game might
     * register its own alternate stack, so we set our own just before the
     * checkpoint and we restore the game's alternate stack just after.
     */
    AltStack::prepareStack();

    /* All other threads halted in 'stopThisThread' routine (they are all
     * in state ST_SUSPENDED).  It's safe to write checkpoint file now.
     * We call the write function with a signal, so we can use an alternate stack
     * and safely writing to the original stack.
     */

    raise(SIGUSR2);

    /* Restoring the game alternate stack (if any) */
    AltStack::restoreStack();

    resumeThreads();

    /* Restoring the original signal handlers */
    CustomSignals::restoreHandlers();

    /* Wait for all other threads to finish being restored before resuming */
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Waiting for other threads to resume");
    waitForAllRestored(current_thread);
    debuglog(LCF_THREAD | LCF_CHECKPOINT, "Resuming main thread");

    ThreadSync::releaseLocks();
}

void ThreadManager::restore()
{
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)
    ThreadSync::acquireLocks();

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
    AudioPlayer::close();

    /* Perform a series of checks before attempting to restore */
    if (!Checkpoint::checkRestore()) {
        ThreadSync::releaseLocks();
        return;
    }

    /* Before suspending threads, we must also register our signal handlers */
    CustomSignals::registerHandlers();

    /* We save the alternate stack if the game did set one */
    AltStack::saveStack();

    restoreInProgress = false;

    suspendThreads();

    restoreInProgress = true;

    /* We set the alternate stack to our reserved memory. The game might
     * register its own alternate stack, so we set our own just before the
     * checkpoint and we restore the game's alternate stack just after.
     */
    AltStack::prepareStack();

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

     /* Restoring the game alternate stack (if any) */
     AltStack::restoreStack();

     resumeThreads();

     /* Restoring the original signal handlers */
     CustomSignals::restoreHandlers();

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
            debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Signaling thread %d", thread->tid);
            next = thread->next;
            int ret;

            /* Do various things based on thread's state */
            switch (thread->state) {
            case ThreadInfo::ST_RUNNING:
            case ThreadInfo::ST_ZOMBIE:
            case ThreadInfo::ST_FREE:

                /* Thread is running. Send it a signal so it will call stopthisthread.
                * We will need to rescan (hopefully it will be suspended by then)
                */
                thread->orig_state = thread->state;
                if (updateState(thread, ThreadInfo::ST_SIGNALED, thread->state)) {

                    /* Setup an alternate signal stack.
                     *
                     * This is a workaround for a bug when loading a savestate
                     * which involves the stack pointer.
                     * During a state loading, the main thread restores the
                     * memory of the thread stacks, then resume the threads, and
                     * each thread restore its registers.
                     * If the stack pointer had changed, the thread is resumed
                     * with a corrupted stack (old stack pointer, new stack memory)
                     * and cannot reach the function to restore its stack pointer.
                     *
                     * The workaround is to run our signal handler function on
                     * an alternate stack (different for each thread).
                     * This way, this stack pointer will be the same.
                     */

                    // NATIVECALL(ret = sigaltstack(&thread->altstack, nullptr));
                    // if (ret < 0) {
                    //     debuglog(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "sigaltstack failed with error ", ret);
                    // }
                    // else {
                    //     debuglog(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "sigaltstack with add ", thread->altstack.ss_sp);
                    // }
                    // struct sigaction sigusr1;
                    // sigfillset(&sigusr1.sa_mask);
                    // sigusr1.sa_flags = SA_RESTART | SA_ONSTACK;
                    // sigusr1.sa_handler = stopThisThread;
                    // {
                    //     GlobalNative gn;
                    //     MYASSERT(sigaction(SIGUSR1, &sigusr1, nullptr) == 0)
                    // }

                    /* Send the suspend signal to the thread */
                    NATIVECALL(ret = pthread_kill(thread->pthread_id, SIGUSR1));

                    if (ret == 0) {
                        needrescan = true;
                    }
                    else {
                        MYASSERT(ret == ESRCH)
                        debuglog(LCF_THREAD | LCF_CHECKPOINT, "Thread", thread->tid, "has died since");
                        threadIsDead(thread);
                    }
                }
                break;

            case ThreadInfo::ST_SIGNALED:
                NATIVECALL(ret = pthread_kill(thread->pthread_id, 0));

                if (ret == 0) {
                    needrescan = true;
                }
                else {
                    MYASSERT(ret == ESRCH)
                    debuglog(LCF_ERROR | LCF_THREAD | LCF_CHECKPOINT, "Signalled thread ", thread->tid, " died");
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

            // case ThreadInfo::ST_FAKEZOMBIE:
            //     break;

            case ThreadInfo::ST_UNINITIALIZED:
                break;

            case ThreadInfo::ST_RECYCLED:
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
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Received suspend signal!");
    if (current_thread->state == ThreadInfo::ST_CKPNTHREAD) {
        return;
    }

    /* Checking that we run in our custom stack, using the address of a local variable */
    stack_t altstack = current_thread->altstack;
    if ((&altstack < altstack.ss_sp) ||
        (&altstack >= (void*)((char*)altstack.ss_sp + altstack.ss_size))) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Thread suspend is not running on alternate stack");
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Local variable in %p and stack at %p", &altstack, altstack.ss_sp);
    }

    /* Make sure we don't get called twice for same thread */
    if (updateState(current_thread, ThreadInfo::ST_SUSPINPROG, ThreadInfo::ST_SIGNALED)) {

        /* Set up our restart point, ie, we get jumped to here after a restore */

        ThreadLocalStorage::saveTLSState(&current_thread->tlsInfo); // save thread local storage
        MYASSERT(getcontext(&current_thread->savctx) == 0)

        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread after getcontext");

        if (!restoreInProgress) {

            /* We are a user thread and all context is saved.
             * Wait for ckpt thread to write ckpt, and resume.
             */

            /* Tell the checkpoint thread that we're all saved away */
            MYASSERT(updateState(current_thread, ThreadInfo::ST_SUSPENDED, ThreadInfo::ST_SUSPINPROG))
            sem_post(&semNotifyCkptThread);

            /* Then wait for the ckpt thread to write the ckpt file then wake us up */
            debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread suspended");

            // sigset_t mask;
            // sigemptyset(&mask);
            // sigaddset(&mask, SIGTRAP);
            // NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));
            // raise(SIGTRAP);

            MYASSERT(pthread_rwlock_rdlock(&threadResumeLock) == 0)
            MYASSERT(pthread_rwlock_unlock(&threadResumeLock) == 0)

            // raise(SIGTRAP);

            /* If when thread was suspended, we performed a restore,
             * then we must resume execution using setcontext
             */
            if (restoreInProgress) {
                setcontext(&current_thread->savctx);
                /* NOT REACHED */
            }
        }
        else {
            ThreadLocalStorage::restoreTLSState(&current_thread->tlsInfo); // restore thread local storage
        }

        MYASSERT(updateState(current_thread, current_thread->orig_state, ThreadInfo::ST_SUSPENDED))

        /* We successfully resumed the thread. We wait for all other
         * threads to restore before continuing
         */
        waitForAllRestored(current_thread);

        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread returning to user code");
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

}
