/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "SaveStateManager.h"
#include "ThreadManager.h"
#include "ThreadSync.h"
#include "Checkpoint.h"
#include "AltStack.h"
#include "ReservedMemory.h"
#include "ThreadInfo.h"

#include "general/timewrappers.h" // clock_gettime
#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#ifdef __linux__
#include "fileio/URandom.h"
#include "audio/AudioPlayerAlsa.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "audio/AudioPlayerCoreAudio.h"
#endif
#include "fileio/FileHandleList.h"
#include "fileio/SaveFileList.h"
#include "renderhud/MessageWindow.h"
#ifdef __unix__
#include "xcb/xcbconnection.h" // x11::gameConnections
#include "xlib/xdisplay.h" // x11::gameDisplays
#endif

#include <sstream>
#include <utility>
#include <csignal>
#include <algorithm> // std::find
#include <sys/mman.h>
#include <sys/syscall.h> // syscall, SYS_gettid
#include <sys/wait.h> // waitpid
#ifdef __unix__
#include <xcb/xproto.h> // xcb_get_input_focus_reply, xcb_get_input_focus
#include <X11/Xlib.h> // XLockDisplay
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

/* From DMTCP */
static void save_sp(void **sp)
{
#if defined(__i386__)
    asm volatile ("mov %%esp, %0"
                  : "=g" (*sp)
                    : : "memory");
#elif defined(__x86_64__)
    asm volatile ("mov %%rsp, %0"
                    : "=g" (*sp)
                    : : "memory");
#elif defined(__arm__) || defined(__aarch64__)
    asm volatile ("mov %0,sp"
                  : "=r" (*sp)
                  : : "memory");
#else
# error "assembly instruction not translated"
#endif
}

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
    sigcheckpoint.sa_flags = SA_ONSTACK | SA_SIGINFO;
    sigcheckpoint.sa_sigaction = Checkpoint::handler;
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

    struct sigaction sigsuspend;
    sigfillset(&sigsuspend.sa_mask);
    sigsuspend.sa_flags = SA_RESTART;
    sigsuspend.sa_handler = stopThisThread;
    {
        GlobalNative gn;
        MYASSERT(sigaction(sig_suspend_threads, &sigsuspend, nullptr) == 0)
    }
}

int SaveStateManager::waitChild()
{
    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_FORK))
        return -1;

    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid <= 0) {
        return -1;
    }
    status = WEXITSTATUS(status);
    if ((status < 0) && (status > 10)) {
        LOG(LL_ERROR, LCF_CHECKPOINT, "Got unknown status code %d from pid %d", status, pid);
        return -1;
    }
    if (!state_dirty[status]) {
        LOG(LL_ERROR, LCF_CHECKPOINT, "State saving %d completed but was already ready", status);
        return -1;
    }

    state_dirty[status] = false;
    return status;
}

bool SaveStateManager::stateReady(int slot)
{
    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_FORK))
        return true;

    if ((slot < 0) && (slot > 10)) {
        LOG(LL_ERROR, LCF_CHECKPOINT, "Wrong slot number");
        return false;
    }
    return !state_dirty[slot];
}

void SaveStateManager::stateStatus(int slot, bool dirty)
{
    if (Global::shared_config.savestate_settings & SharedConfig::SS_FORK)
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
            NATIVECALL(XLockDisplay(x11::gameDisplays[i]));
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

    /* Update thread stack address and size for stack padding.
     * Must be done **before** suspending threads. */
    ThreadManager::updateStackInfo();

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

    /* Map all savefiles to memory, so that they will be saved into savestates */ 
    SaveFileList::mapFiles();

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

    /* Important note: while we are in the function to save a state, the remaining
     * part of this function is for **both** state saving and state loading!
     * This is because state loading recovers the exact context that was saved,
     * so it is logical that we resume execution here. To differentiate between
     * state saving and state loading, the signal handled above set the variable
     * `restoreInProgress` if loading a state. So we can use this variable to 
     * execute specific code. */

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

    /* Create threads that were destroyed since the savestate */
    if (restoreInProgress) {
        createNewThreads();
    }

    resumeThreads();

#ifdef __unix__
    /* After restore, we need to sync xcb connections to avoid potential deadlocks */
    if (restoreInProgress) {
        for (int i=0; i<GAMECONNECTIONNUM; i++) {
            if (x11::gameConnections[i])
                NATIVECALL(free(xcb_get_input_focus_reply(x11::gameConnections[i], xcb_get_input_focus(x11::gameConnections[i]), nullptr)));
        }
    }

    /* Unlock the display */
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i])
            NATIVECALL(XUnlockDisplay(x11::gameDisplays[i]));
    }
#endif

    /* Wait for all other threads to finish being restored before resuming */
    LOG(LL_DEBUG, LCF_CHECKPOINT, "Waiting for other threads to resume");
    waitForAllRestored(current_thread);
    LOG(LL_DEBUG, LCF_CHECKPOINT, "Resuming main thread");

    ThreadSync::releaseLocks();

    /* Mark the savestate as dirty in case of fork savestate */
    if (!restoreInProgress)
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

    /* `restoreInProgress` is set to false first, so that we can suspend threads
     * like in SaveStateManager::checkpoint() */
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

    /* Stop threads that will not be present after state loading */
    terminateThreads();
    
    suspendThreads();

    /* Save thread list in reserved memory */
    saveThreadList();

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

    /* The kernel will store the current ucontext onto the stack the signal
     * handler uses. This is the non-savestable alt stack. We ensure that
     * a copy of this ucontext is placed onto savestable memory, and use that
     * in order to restore the ucontext on a load state. This ensures that on
     * returning, we will return to after the `raise(sig_checkpoint)` call in
     * ThreadManager::checkpoint(). We don't even need to use getcontext/setcontext!
     */

     /* If restore was not done, we return here */
     LOG(LL_DEBUG, LCF_CHECKPOINT, "Restoring was not done, resuming threads");

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

void SaveStateManager::terminateThreads()
{
    StateHeader sh;
    Checkpoint::getStateHeader(&sh);
    
    ThreadManager::lockList();
    
    /* Compare the two lists of threads and flag the ones to be terminated */
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if (thread->state != ThreadInfo::ST_RUNNING)
            continue;
        
        int t;
        for (t=0; t<sh.thread_count; t++) {
            if (sh.tids[t] == thread->translated_tid) {
                break;
            }
        }
        
        if (t == sh.thread_count) {
            /* Thread was not found in savestate, flag it */
            thread->state = ThreadInfo::ST_TERMINATING;
        }
    }

    ThreadManager::unlockList();
}

void SaveStateManager::suspendThreads()
{
    MYASSERT(pthread_mutex_lock(&threadResumeLock) == 0)

    /* Terminate threads flagged as such.
     * Halt all other threads - force them to call stopthisthread
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
            next = thread->next;
            int ret;

            /* Do various things based on thread's state */
            switch (thread->state) {
            case ThreadInfo::ST_ZOMBIE:
                /* Zombie threads don't need to be signaled, because as they are
                 * detached by us from the beginning, the underlying linux thread
                 * should have exited. We still need to save if threads are recycled */
                break;

            case ThreadInfo::ST_RUNNING:
                /* Thread is running. Send it a signal so it will call stopthisthread.
                * We will need to rescan (hopefully it will be suspended by then)
                */
                thread->orig_state = thread->state;
                if (ThreadManager::updateState(thread, ThreadInfo::ST_SIGNALED, ThreadInfo::ST_RUNNING)) {
                    /* Send the suspend signal to the thread */
                    LOG(LL_DEBUG, LCF_CHECKPOINT, "Signaling thread %d", thread->real_tid);
                    NATIVECALL(ret = pthread_kill(thread->pthread_id, sig_suspend_threads));

                    if (ret == 0) {
                        needrescan = true;
                    }
                    else {
                        MYASSERT(ret == ESRCH)
                        LOG(LL_DEBUG, LCF_CHECKPOINT, "Thread %d has died since", thread->real_tid);
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
                    LOG(LL_ERROR, LCF_CHECKPOINT, "Signalled thread %d died", thread->real_tid);
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
                /* This thread, don't signal ourself */
                break;

            case ThreadInfo::ST_UNINITIALIZED:
                /* Thread in the middle of being created (that should not
                 * happen because of locks?), try again */
                needrescan = true;
                break;

            case ThreadInfo::ST_TERMINATING:
                if (ThreadManager::updateState(thread, ThreadInfo::ST_TERMINATED, ThreadInfo::ST_TERMINATING)) {

                    /* Try to cancel the thread */
                    int ret;
                    LOG(LL_DEBUG, LCF_CHECKPOINT, "Cancel thread %d", thread->real_tid);
                    NATIVECALL(ret = pthread_cancel(thread->pthread_id));

                    if (ret < 0) {
                        LOG(LL_ERROR, LCF_CHECKPOINT, "Signalled thread %d died", thread->real_tid);
                        ThreadManager::threadIsDead(thread);
                        break;
                    }

                    /* We must wait until the thread terminates entirely, because
                     * it may release resources during state loading.
                     * A terminating thread sets the tid in pthread struct to 0 */
                    int i = 0;
                    while (*thread->ptid != 0) {
                        LOG(LL_DEBUG, LCF_CHECKPOINT, "Wait for tid %d to become 0", *thread->ptid);                        
                        NATIVECALL(usleep(1000));
                        i++;
                        if (i > 1000)
                            break;
                    }
                    
                    if (*thread->ptid != 0) {
                        /* If we couldn't cancel the thread, try to signal it so that
                        * it can call pthread_exit(). This is unsafe! */
                        LOG(LL_DEBUG, LCF_CHECKPOINT, "Cancel failed, signaling thread %d to terminate", thread->real_tid);
                        NATIVECALL(ret = pthread_kill(thread->pthread_id, sig_suspend_threads));
                        
                        while (*thread->ptid != 0) {
                            LOG(LL_DEBUG, LCF_CHECKPOINT, "Wait for tid %d to become 0", *thread->ptid);                        
                            NATIVECALL(usleep(1000));
                        }
                    }
                }
                break;

            case ThreadInfo::ST_TERMINATED:
                break;

            default:
                LOG(LL_ERROR, LCF_CHECKPOINT, "Unknown thread state %d", thread->state);
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

    LOG(LL_DEBUG, LCF_CHECKPOINT, "%d threads were suspended", numThreads);
}

/* Resume all threads. */
void SaveStateManager::resumeThreads()
{
    LOG(LL_DEBUG, LCF_CHECKPOINT, "Resuming all threads");
    MYASSERT(pthread_mutex_unlock(&threadResumeLock) == 0)
    LOG(LL_DEBUG, LCF_CHECKPOINT, "All threads resumed");
}

void SaveStateManager::stopThisThread(int signum)
{
    ThreadInfo *current_thread = ThreadManager::getCurrentThread();

    if (current_thread->state == ThreadInfo::ST_TERMINATED) {
        NATIVECALL(pthread_exit(nullptr));
    }
    
    if (current_thread->state == ThreadInfo::ST_CKPNTHREAD) {
        return;
    }

    /* Make sure we don't get called twice for same thread */
    if (ThreadManager::updateState(current_thread, ThreadInfo::ST_SUSPINPROG, ThreadInfo::ST_SIGNALED)) {

        /* Set up our restart point, ie, we get jumped to here after a restore */

        ThreadLocalStorage::saveTLSState(&current_thread->tlsInfo); // save thread local storage
        MYASSERT(getcontext(&current_thread->savctx) == 0)
        
        /* We save the current stack pointer, so that we can use the remaining
         * space in the stack as a stack segment for the clone() call if the current
         * thread has to be created */
        save_sp(&current_thread->saved_sp);

        if (!restoreInProgress) {

            /* Both state saving and state loading operations go here first, to
             * suspend the thread. */

            /* Tell the checkpoint thread that we're all saved away */
            MYASSERT(ThreadManager::updateState(current_thread, ThreadInfo::ST_SUSPENDED, ThreadInfo::ST_SUSPINPROG))
            NATIVECALL(sem_post(&semNotifyCkptThread));

            /* Then wait for the ckpt thread to write the ckpt file then wake us up */
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Thread suspended");

            /* Before suspending, we must position the stack pointer so that it
             * always takes the same value. Because the stack pointer may not be
             * the same between saving and loading the thread, and so returning
             * after the `pthread_mutex_lock()` will cause a crash.
             * To do that, we allocate on the stack a big array so that we reach
             * near the end of the stack. */
            ptrdiff_t free_space = reinterpret_cast<ptrdiff_t>(current_thread->saved_sp)
                                 - reinterpret_cast<ptrdiff_t>(current_thread->stack_addr);
            free_space -= 4096*4; // Arbitrary size left to run the remaining code

            LOG(LL_DEBUG, LCF_CHECKPOINT, "Current stack pointer %p and base stack %p size %zu, allocate %td bytes", current_thread->saved_sp, current_thread->stack_addr, current_thread->stack_size, free_space);
            if (free_space < 0) {
                LOG(LL_WARN, LCF_CHECKPOINT, "We don't have enough space left in the stack to pad");
            }
            else {
                uintptr_t *pad = static_cast<uintptr_t*>(alloca(free_space));
                pad[0] = reinterpret_cast<uintptr_t>(pad); // to hopefully prevent optimization
            }

            /* We save again the thread struct on the stack at this now safe
             * position, so that we will be able to use it after resuming */
            ThreadInfo *current_thread_safe = ThreadManager::getCurrentThread();

            /* We use `pthread_mutex_lock()` to suspend a thread because it is
             * the most pure sync mechanism underneath, which should translate 
             * mostly has a futex syscall. Thanks to that, the thread stack can
             * be modified by the state loading code without it being bothered */
            MYASSERT(pthread_mutex_lock(&threadResumeLock) == 0)
            MYASSERT(pthread_mutex_unlock(&threadResumeLock) == 0)

            /* After the thread is resumed from loading a state, we immediately
             * use `setcontext()` to resume execution. `restoreInProgress` was
             * modified by the checkpoint thread during state loading, when this
             * thread was suspended
             */
            if (restoreInProgress) {
                
                setcontext(&current_thread_safe->savctx);
                /* NOT REACHED */
            }
        }
        else {
            /* After loading a state and recovering the thread context, we can
             * restore the thread local storage */
            ThreadLocalStorage::restoreTLSState(&current_thread->tlsInfo);
            /* Recover the real tid */
            ThreadManager::restoreTid();
        }

        MYASSERT(ThreadManager::updateState(current_thread, current_thread->orig_state, ThreadInfo::ST_SUSPENDED))

        /* We successfully resumed the thread. We wait for all other
         * threads to restore before continuing
         */
        waitForAllRestored(current_thread);

        LOG(LL_DEBUG, LCF_CHECKPOINT, "Thread returning to user code");
    }
}

void SaveStateManager::saveThreadList()
{
    /* The thread list is saved in the reserved memory segments, meaning that
     * it will be unmodified by state loading */
    StateHeader* sh = static_cast<StateHeader*>(ReservedMemory::getAddr(ReservedMemory::SH_ADDR));

    int n=0;
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED) {
            if (n >= STATEMAXTHREADS) {
                LOG(LL_ERROR, LCF_CHECKPOINT, "   hit the limit of the number of threads");
                break;
            }
            sh->pthread_ids[n] = thread->pthread_id;
            sh->tids[n] = thread->translated_tid;
            sh->states[n++] = thread->orig_state;
        }
    }
    sh->thread_count = n;
}

void SaveStateManager::createNewThreads()
{
    /* Recover the thread list from before state loading, to compare with the
     * actual list */
    StateHeader* sh = static_cast<StateHeader*>(ReservedMemory::getAddr(ReservedMemory::SH_ADDR));
    
    /* Compare the two lists of threads and create the missing threads */
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if (!(thread->state == ThreadInfo::ST_SUSPENDED))
            continue;
        
        int t;
        for (t=0; t<sh->thread_count; t++) {
            if (sh->tids[t] == thread->translated_tid) {
                break;
            }
        }
        
        if (t == sh->thread_count) {
            /* Thread was not found, create one */
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_THREAD, "Recreate thread %llx (%d) ", thread->pthread_id, thread->translated_tid);

            int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM |
                        CLONE_SIGHAND | CLONE_THREAD |
                        CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
            NATIVECALL(thread->real_tid = clone(startNewThread,
                        // -128 for red zone
                        (void *)((char *)thread->saved_sp - 128),
                        flags, thread, thread->ptid, NULL, thread->ptid));
        }
    }
}

int SaveStateManager::startNewThread(void *arg)
{
    ThreadInfo *thread = static_cast<ThreadInfo*>(arg);
    
    /* Wait for the checkpoint thread to resume all threads.
     * We need to wait so that the checkpoint thread resumes the correct count 
     * of threads. */
    MYASSERT(pthread_mutex_lock(&threadResumeLock) == 0)
    MYASSERT(pthread_mutex_unlock(&threadResumeLock) == 0)

    /* Restore the saved thread context, and execution will resume inside stopThisThread() */
    setcontext(&thread->savctx);
    
    /* Not reached */
    return 0;
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
        LOG(LL_ERROR, LCF_CHECKPOINT, errors[-err]);
        MessageWindow::insert(errors[-err]);
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
