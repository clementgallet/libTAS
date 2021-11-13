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

    Part of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */
#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include <sys/syscall.h> // syscall, SYS_gettid
#include <sys/wait.h> // waitpid
#ifdef __unix__
#include <X11/Xlib.h> // XLockDisplay
#endif

#include "SaveStateManager.h"
#include "ThreadManager.h"
#include "ThreadSync.h"
#include "Checkpoint.h"
#include "../timewrappers.h" // clock_gettime
#include "../logging.h"
#ifdef __linux__
#include "../fileio/URandom.h"
#include "../audio/AudioPlayerAlsa.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "../audio/AudioPlayerCoreAudio.h"
#endif
#include "AltStack.h"
#include "ReservedMemory.h"
#include "../fileio/FileHandleList.h"
#include "../renderhud/RenderHUD.h"
#ifdef __unix__
#include "../xlib/xdisplay.h" // x11::gameDisplays
#endif

namespace libtas {

static sem_t semNotifyCkptThread;
static sem_t semWaitForCkptThreadSignal;
static pthread_mutex_t threadResumeLock = PTHREAD_MUTEX_INITIALIZER;
static volatile bool restoreInProgress = false;
static int numThreads;
static int sig_suspend_threads = SIGXFSZ;
static int sig_checkpoint = SIGSYS;
static bool* state_dirty;

int SaveStateManager::sigCheckpoint()
{
    return sig_checkpoint;
}

int SaveStateManager::sigSuspend()
{
    return sig_suspend_threads;
}

void SaveStateManager::init()
{
    sem_init(&semNotifyCkptThread, 0, 0);
    sem_init(&semWaitForCkptThreadSignal, 0, 0);

    ReservedMemory::init();

    state_dirty = static_cast<bool*>(ReservedMemory::getAddr(ReservedMemory::SS_SLOTS_ADDR));
    memset(state_dirty, 0, 11*sizeof(bool));
}

void SaveStateManager::initCheckpointThread()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig_checkpoint);
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));

    struct sigaction sigcheckpoint;
    sigfillset(&sigcheckpoint.sa_mask);
    sigcheckpoint.sa_flags = SA_ONSTACK;
    sigcheckpoint.sa_handler = Checkpoint::handler;
    {
        GlobalNative gn;
        MYASSERT(sigaction(sig_checkpoint, &sigcheckpoint, nullptr) == 0)
    }
}

void SaveStateManager::initThreadFromChild(ThreadInfo* thread)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig_suspend_threads);
    NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));

    /* The signal handler for the thread suspend must be registered on *each*
     * thread, because it runs on a custom stack.
     */
    if (!thread->altstack.ss_sp) {
        thread->altstack.ss_size = 64*1024;
        thread->altstack.ss_sp = malloc(thread->altstack.ss_size);
        thread->altstack.ss_flags = 0;
    }

    int ret;
    NATIVECALL(ret = sigaltstack(&thread->altstack, nullptr));
    if (ret < 0) {
        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "sigaltstack failed with error %d", errno);
        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "stack starts at %p", thread->altstack.ss_sp);
    }

    struct sigaction sigsuspend;
    sigfillset(&sigsuspend.sa_mask);
    sigsuspend.sa_flags = SA_RESTART | SA_ONSTACK;
    sigsuspend.sa_handler = stopThisThread;
    {
        GlobalNative gn;
        MYASSERT(sigaction(sig_suspend_threads, &sigsuspend, nullptr) == 0)
    }
}

int SaveStateManager::waitChild()
{
    if (!(shared_config.savestate_settings & SharedConfig::SS_FORK))
        return -1;

    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid <= 0) {
        return -1;
    }
    status = WEXITSTATUS(status);
    if ((status < 0) && (status > 10)) {
        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "Got unknown status code %d from pid %d", status, pid);
        return -1;
    }
    if (!state_dirty[status]) {
        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "State saving %d completed but was already ready", status);
        return -1;
    }

    state_dirty[status] = false;
    return status;
}

bool SaveStateManager::stateReady(int slot)
{
    if (!(shared_config.savestate_settings & SharedConfig::SS_FORK))
        return true;

    if ((slot < 0) && (slot > 10)) {
        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT | LCF_ERROR, "Wrong slot number");
        return false;
    }
    return !state_dirty[slot];
}

void SaveStateManager::stateStatus(int slot, bool dirty)
{
    if (shared_config.savestate_settings & SharedConfig::SS_FORK)
        state_dirty[slot] = dirty;
}

int SaveStateManager::checkpoint(int slot)
{
    if (!stateReady(slot))
        return ESTATE_NOTCOMPLETE;

    ThreadInfo *current_thread = ThreadManager::getCurrentThread();
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)

    ThreadSync::acquireLocks();

    restoreInProgress = false;

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
#ifdef __linux__
    AudioPlayerAlsa::close();
#elif defined(__APPLE__) && defined(__MACH__)
    AudioPlayerCoreAudio::close();
#endif

    /* Perform a series of checks before attempting to checkpoint */
    int ret = Checkpoint::checkCheckpoint();
    if (ret < 0) {
        ThreadSync::releaseLocks();
        return ret;
    }

    /* We save the alternate stack if the game did set one */
    AltStack::saveStack();

#ifdef __unix__
    /* Lock the display so we can empty events */
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            XLockDisplay(x11::gameDisplays[i]);
    }
#endif

    /* Sync all X server connections. Must be done before suspending threads,
     * because while we have locked the displays, threads could still be suspended
     * inside xcb functions that temporarily unlock the Xlib lock, and we will
     * softlock because of xcb lock.
     *
     * Real example from flashplayer:
     * - thread 2 holds the xcb lock and is suspended:
     *     libtas::SaveStateManager::stopThisThread(int) (signum=<optimized out>)
     *     <signal handler called> () at /lib/x86_64-linux-gnu/libpthread.so.0
     *     ...
     *     xcb_wait_for_reply64 () at /usr/lib/x86_64-linux-gnu/libxcb.so.1
     *     _XReply () at /usr/lib/x86_64-linux-gnu/libX11.so.6
     *     XGetGeometry () at /usr/lib/x86_64-linux-gnu/libX11.so.6
     *
     * - thread 1 waits on the xcb lock while emptying the event queue before savestating:
     *     __lll_lock_wait (futex=futex@entry=0x22ef1d8, private=0) at lowlevellock.c:52
     *     __GI___pthread_mutex_lock (mutex=0x22ef1d8) at ../nptl/pthread_mutex_lock.c:80
     *     xcb_writev () at /usr/lib/x86_64-linux-gnu/libxcb.so.1
     *     _XSend () at /usr/lib/x86_64-linux-gnu/libX11.so.6
     *     _XReply () at /usr/lib/x86_64-linux-gnu/libX11.so.6
     *     XSync () at /usr/lib/x86_64-linux-gnu/libX11.so.6
     */
#ifdef __unix__
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            NATIVECALL(XSync(x11::gameDisplays[i], false));
    }
#endif

    /* Sending a suspend signal to all threads */
    suspendThreads();

#ifdef __linux__
    /* Disable the signal that refills the fake urandom pipe. Must be done
     * before the file tracking
     */
    urandom_disable_handler();
#endif

    /* We flag all opened files as tracked and store their offset. This must be
     * done AFTER suspending threads.
     */
    FileHandleList::trackAllFiles();

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

    NATIVECALL(raise(sig_checkpoint));

    /* Restoring the game alternate stack (if any) */
    AltStack::restoreStack();

    /* We recover the offset of all opened files. This must also be done BEFORE
     * resuming threads.
     */
    FileHandleList::recoverAllFiles();

#ifdef __linux__
    /* Restore the signal that refills the fake urandom pipe */
    urandom_enable_handler();
#endif

    resumeThreads();

#ifdef __unix__
    /* Unlock the display */
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            XUnlockDisplay(x11::gameDisplays[i]);
    }
#endif

    /* Wait for all other threads to finish being restored before resuming */
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Waiting for other threads to resume");
    waitForAllRestored(current_thread);
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Resuming main thread");

    ThreadSync::releaseLocks();

    /* Mark the savestate as dirty in case of fork savestate */
    if (!isLoading())
        stateStatus(slot, true);

    return ESTATE_OK;
}

int SaveStateManager::restore(int slot)
{
    if (!stateReady(slot))
        return ESTATE_NOTCOMPLETE;

    ThreadInfo *current_thread = ThreadManager::getCurrentThread();
    MYASSERT(current_thread->state == ThreadInfo::ST_CKPNTHREAD)
    ThreadSync::acquireLocks();

    /* We must close the connection to the sound device. This must be done
     * BEFORE suspending threads.
     */
#ifdef __linux__
    AudioPlayerAlsa::close();
#elif defined(__APPLE__) && defined(__MACH__)
    AudioPlayerCoreAudio::close();
#endif

    /* Perform a series of checks before attempting to restore */
    int ret = Checkpoint::checkRestore();
    if (ret < 0) {
        ThreadSync::releaseLocks();
        return ret;
    }

    /* We save the alternate stack if the game did set one */
    AltStack::saveStack();

    restoreInProgress = false;

#ifdef __unix__
    /* Lock the display so we can empty events */
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            XLockDisplay(x11::gameDisplays[i]);
    }

    /* Sync all X server connections. */
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            NATIVECALL(XSync(x11::gameDisplays[i], false));
    }
#endif

    suspendThreads();

    restoreInProgress = true;

#ifdef __linux__
    /* Disable the signal that refills the fake urandom pipe. */
    urandom_disable_handler();
#endif

    /* We close all untracked files, because by definition they are closed when
     * the savestate will be loaded. */
    FileHandleList::closeUntrackedFiles();

    /* We set the alternate stack to our reserved memory. The game might
     * register its own alternate stack, so we set our own just before the
     * checkpoint and we restore the game's alternate stack just after.
     */
    AltStack::prepareStack();

    /* Here is where we load all the memory and stuff */
    NATIVECALL(raise(sig_checkpoint));

    /* It seems that when restoring the original stack at the end of the
     * signal handler function, the program pulls from the stack the address
     * to return to. Because we replace the stack memory with the checkpointed
     * stack, we will return to after `raise(sig_checkpoint)` call in
     * ThreadManager::checkpoint(). We may even not have to use
     * getcontext/setcontext for this thread!
     */

     /* If restore was not done, we return here */
     debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Restoring was not done, resuming threads");

     /* Restoring the game alternate stack (if any) */
     AltStack::restoreStack();

#ifdef __linux__
     /* Restore the signal that refills the fake urandom pipe */
     urandom_enable_handler();
#endif

     resumeThreads();

#ifdef __unix__
     /* Unlock the display */
     for (int i=0; i<GAMEDISPLAYNUM; i++) {
         if (x11::gameDisplays[i])
             XUnlockDisplay(x11::gameDisplays[i]);
     }
#endif

     waitForAllRestored(current_thread);

     ThreadSync::releaseLocks();

     return ESTATE_UNKNOWN;
}

void SaveStateManager::suspendThreads()
{
    MYASSERT(pthread_mutex_destroy(&threadResumeLock) == 0)
    MYASSERT(pthread_mutex_init(&threadResumeLock, NULL) == 0)
    MYASSERT(pthread_mutex_lock(&threadResumeLock) == 0)

    /* Halt all other threads - force them to call stopthisthread
    * If any have blocked checkpointing, wait for them to unblock before
    * signalling
    */
    ThreadManager::lockList();

    bool needrescan = false;
    do {
        needrescan = false;
        numThreads = 0;
        ThreadInfo *next;
        for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = next) {
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
                if (ThreadManager::updateState(thread, ThreadInfo::ST_SIGNALED, thread->state)) {

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
                    NATIVECALL(ret = pthread_kill(thread->pthread_id, sig_suspend_threads));

                    if (ret == 0) {
                        needrescan = true;
                    }
                    else {
                        MYASSERT(ret == ESRCH)
                        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread %d has died since", thread->tid);
                        ThreadManager::threadIsDead(thread);
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
                    debuglogstdio(LCF_ERROR | LCF_THREAD | LCF_CHECKPOINT, "Signalled thread %d died", thread->tid);
                    ThreadManager::threadIsDead(thread);
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
                debuglogstdio(LCF_ERROR | LCF_THREAD | LCF_CHECKPOINT, "Unknown thread state %d", thread->state);
            }
        }
        if (needrescan) {
            struct timespec sleepTime = { 0, 10 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
        }
    } while (needrescan);

    ThreadManager::unlockList();

    for (int i = 0; i < numThreads; i++) {
        NATIVECALL(sem_wait(&semNotifyCkptThread));
    }

    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "%d threads were suspended", numThreads);
}

/* Resume all threads. */
void SaveStateManager::resumeThreads()
{
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Resuming all threads");
    MYASSERT(pthread_mutex_unlock(&threadResumeLock) == 0)
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "All threads resumed");
}

void SaveStateManager::stopThisThread(int signum)
{
    debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Received suspend signal!");

    ThreadInfo *current_thread = ThreadManager::getCurrentThread();
    if (current_thread->state == ThreadInfo::ST_CKPNTHREAD) {
        return;
    }

    /* Checking that we run in our custom stack, using the address of a local variable */
    stack_t altstack = current_thread->altstack;
    if ((&altstack < altstack.ss_sp) ||
        (&altstack >= (void*)((char*)altstack.ss_sp + altstack.ss_size))) {
        debuglogstdio(LCF_CHECKPOINT | LCF_WARNING, "Thread suspend is not running on alternate stack");
        debuglogstdio(LCF_CHECKPOINT | LCF_WARNING, "Local variable in %p and stack at %p", &altstack, altstack.ss_sp);
    }

    /* Make sure we don't get called twice for same thread */
    if (ThreadManager::updateState(current_thread, ThreadInfo::ST_SUSPINPROG, ThreadInfo::ST_SIGNALED)) {

        /* Set up our restart point, ie, we get jumped to here after a restore */

        ThreadLocalStorage::saveTLSState(&current_thread->tlsInfo); // save thread local storage
        MYASSERT(getcontext(&current_thread->savctx) == 0)

        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread after getcontext");

        if (!restoreInProgress) {

            /* We are a user thread and all context is saved.
             * Wait for ckpt thread to write ckpt, and resume.
             */

            /* Tell the checkpoint thread that we're all saved away */
            MYASSERT(ThreadManager::updateState(current_thread, ThreadInfo::ST_SUSPENDED, ThreadInfo::ST_SUSPINPROG))
            NATIVECALL(sem_post(&semNotifyCkptThread));

            /* Then wait for the ckpt thread to write the ckpt file then wake us up */
            debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread suspended");

            // sigset_t mask;
            // sigemptyset(&mask);
            // sigaddset(&mask, SIGTRAP);
            // NATIVECALL(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr));
            // raise(SIGTRAP);

            MYASSERT(pthread_mutex_lock(&threadResumeLock) == 0)
            MYASSERT(pthread_mutex_unlock(&threadResumeLock) == 0)

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

        MYASSERT(ThreadManager::updateState(current_thread, current_thread->orig_state, ThreadInfo::ST_SUSPENDED))

        /* We successfully resumed the thread. We wait for all other
         * threads to restore before continuing
         */
        waitForAllRestored(current_thread);

        debuglogstdio(LCF_THREAD | LCF_CHECKPOINT, "Thread returning to user code");
    }
}

void SaveStateManager::waitForAllRestored(ThreadInfo *thread)
{
    if (thread->state == ThreadInfo::ST_CKPNTHREAD) {
        for (int i = 0; i < numThreads; i++) {
            NATIVECALL(sem_wait(&semNotifyCkptThread));
        }

        /* If this was last of all, wake everyone up */
        for (int i = 0; i < numThreads; i++) {
            NATIVECALL(sem_post(&semWaitForCkptThreadSignal));
        }
    }
    else {
        NATIVECALL(sem_post(&semNotifyCkptThread));
        NATIVECALL(sem_wait(&semWaitForCkptThreadSignal));
    }
}

void SaveStateManager::printError(int err)
{
    const char* const errors[] = {
        "No error",
        "Savestate failed",
        "Not enough available space to store the savestate",
        "Savestate does not exist",
        "Loading not allowed because new threads were created",
        "State still saving",
        0 };

    if (err < 0) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, errors[-err]);
#ifdef LIBTAS_ENABLE_HUD
        RenderHUD::insertMessage(errors[-err]);
#endif
    }
}

bool SaveStateManager::isLoading()
{
    return restoreInProgress;
}

/* Restore the state of loading a savestate, after memory has been rewritten */
void SaveStateManager::setLoading()
{
    restoreInProgress = true;
}

}
