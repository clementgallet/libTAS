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
 */

#include "GameHacks.h"
#include "logging.h"
#include "GlobalState.h"
#ifdef __unix__
#include "checkpoint/ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "checkpoint/MachVmMaps.h"
#endif
#include "checkpoint/MemArea.h"

#include <unistd.h>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace libtas {

static bool unity = false;
static bool coreclr = false;
static pid_t finalizer_pid = 0;

static uintptr_t executableBase = 0;
static uintptr_t executableEnd = 0;
static uintptr_t unityLoadingThreadAddr = 0;

void GameHacks::setUnity()
{
    debuglogstdio(LCF_HOOK, "   detected Unity engine");
    unity = true;
}

bool GameHacks::isUnity()
{
    return unity;
}

void GameHacks::getExecutableMemory()
{
    if (executableBase != 0)
        return;
    
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif
    Area area;
    while (memMapLayout.getNextArea(&area)) {
        if (area.addr == (void*)0x400000 || area.addr == (void*)0x8048000)
            break;
    }
    
    if (!area.addr) {
        debuglogstdio(LCF_ERROR, "Could not detect the game executable memory mapping!");
        /* Put non-zero numbers so that this is not triggered again */
        executableBase = 1;
        executableEnd = 1;
        return;
    }

    executableBase = reinterpret_cast<uintptr_t>(area.addr);
    executableEnd = reinterpret_cast<uintptr_t>(area.endAddr);
}

bool GameHacks::isUnityLoadingThread(uintptr_t addr)
{
    if (!unity) return false;
    
    /* The first Unity thread that executes a routine from its executable seems
     * to always be a loading thread */
    
    getExecutableMemory();
    
    if (unityLoadingThreadAddr != 0)
        return (unityLoadingThreadAddr == addr);

    if ((addr >= executableBase) && (addr < executableEnd)) {
        unityLoadingThreadAddr = addr;
        return true;
    }
    
    return false;
}

static std::mutex unity_mutex;
static std::condition_variable unity_condition;
static unsigned int unity_running_threads = 0;
static unsigned int unity_nonterminating_threads = 0;
static unsigned int unity_waiting_threads = 0;

void GameHacks::unitySyncNotify()
{
    static thread_local bool first = true;
    if (!first) {
        GlobalNative gn;
        std::lock_guard<decltype(unity_mutex)> lock(unity_mutex);
        debuglogstdio(LCF_WAIT, "   Notifty the end of Unity job");
        --unity_running_threads;
        /* TODO: a nonterminating thread that eventually finishes is not supported. */
        unity_condition.notify_all();
    }
    else
        first = false;
}

void GameHacks::unitySyncWait()
{
    GlobalNative gn;
    std::unique_lock<decltype(unity_mutex)> lock(unity_mutex);
    ++unity_waiting_threads;
    debuglogstdio(LCF_WAIT, "   Wait before running a Unity job");
    while (unity_running_threads > unity_nonterminating_threads) {
        
        /* Store the current nonterminating count, so that only one thread that
         * timeouts can increase it */
        unsigned int old_unity_nonterminating_threads = unity_nonterminating_threads;
        
        if (std::cv_status::timeout == unity_condition.wait_for(lock, std::chrono::milliseconds(2000))) {

            /* The current running thread did not finish in time. We consider
            * it as running a nonterminating job, and we increase its count
            * so that one other thread can now run its job. We take extra
            * precaution that exactly one thread will raise the count here
            * by comparing with the old count, to see if another thread did
            * increase it already.
            * TODO: For now, the count can never decrease */

            if ((unity_running_threads > unity_nonterminating_threads) &&
                (unity_nonterminating_threads == old_unity_nonterminating_threads)) {                
                debuglogstdio(LCF_WAIT, "   Increase Unity nonterminating thread count");
                unity_nonterminating_threads++;
            }
        }
    }
    debuglogstdio(LCF_WAIT, "   Start running a Unity job");
    --unity_waiting_threads;
    ++unity_running_threads;
}

void GameHacks::unitySyncWaitAll()
{
    GlobalNative gn;
    
    /* We would normally use the same conditional variable to wait here that
     * all jobs have finished, but it can cause a softlock.
     * It seems the reason is that other threads can send a signal (SIGXCPU) to
     * main thread to trigger a sigsuspend() while it is waiting on conditional
     * variable, and the same thread is responsible for sending a SIGTSTP signal
     * to resume execution. But this thread is waiting for another thread to
     * finish its job, so it never resumes main thread. And the thread that
     * currrently runs the job can never finish, because the conditional
     * variable is locked. This leads to a deadlock.
     
     * Main Thread (that is interrupted from waiting on conditional variable)
     * #0  0x00007ffff797c281 in __GI___sigsuspend (set=0x7fffffffc8f0) at ../sysdeps/unix/sysv/linux/sigsuspend.c:26
     * #1  0x00007ffff7edaae2 in libtas::sigsuspend (set=0x7fffffffc8f0) at general/signalwrappers.cpp:182
     * #2  0x00007fffed1acaf9 in ?? () from /home/clement/Games/Dandara/Dandara_Data/Mono/x86_64/libmono.so
     * #3  <signal handler called> (SIGXCPU)
     * #4  0x00007ffff79c5da4 in __futex_abstimed_wait_common64 (private=0, cancel=true, abstime=0x0, op=393, expected=0, futex_word=0x7ffff7fb12a8 <libtas::unity_condition+40>) at ./nptl/futex-internal.c:57
     * #5  __futex_abstimed_wait_common (futex_word=futex_word@entry=0x7ffff7fb12a8 <libtas::unity_condition+40>, expected=expected@entry=0, clockid=clockid@entry=0, abstime=abstime@entry=0x0, private=private@entry=0, cancel=cancel@entry=true) at ./nptl/futex-internal.c:87
     * #6  0x00007ffff79c5e0b in __GI___futex_abstimed_wait_cancelable64 (futex_word=futex_word@entry=0x7ffff7fb12a8 <libtas::unity_condition+40>, expected=expected@entry=0, clockid=clockid@entry=0, abstime=abstime@entry=0x0, private=private@entry=0) at ./nptl/futex-internal.c:139
     * #7  0x00007ffff79c8468 in __pthread_cond_wait_common (abstime=0x0, clockid=0, mutex=0x7ffff7fb12c0 <libtas::unity_mutex>, cond=0x7ffff7fb1280 <libtas::unity_condition>) at ./nptl/pthread_cond_wait.c:503
     * #8  ___pthread_cond_wait (cond=0x7ffff7fb1280 <libtas::unity_condition>, mutex=0x7ffff7fb12c0 <libtas::unity_mutex>) at ./nptl/pthread_cond_wait.c:618
     * #9  0x00007ffff7ea93ad in libtas::GameHacks::unitySyncWaitAll () at GameHacks.cpp:164
     * #10 0x00007ffff7ea8230 in libtas::frameBoundary(std::function<void ()>, libtas::RenderHUD&) (draw=..., hud=...) at frame.cpp:265
     * #11 0x00007ffff7f0f76d in libtas::SDL_GL_SwapWindow (window=<optimized out>) at sdl/sdlwindows.cpp:102
     * ...
     *
     * Thread 2 (that sent the SIGXCPU signal and is waiting for another job to finish)
     * #0  __futex_abstimed_wait_common64 (private=0, cancel=true, abstime=0x7fffecefce70, op=137, expected=0, futex_word=0x7ffff7fb12ac <libtas::unity_condition+44>) at ./nptl/futex-internal.c:57
     * #1  __futex_abstimed_wait_common (futex_word=futex_word@entry=0x7ffff7fb12ac <libtas::unity_condition+44>, expected=expected@entry=0, clockid=clockid@entry=1, abstime=abstime@entry=0x7fffecefce70, private=private@entry=0, cancel=cancel@entry=true) at ./nptl/futex-internal.c:87
     * #2  0x00007ffff79c5e0b in __GI___futex_abstimed_wait_cancelable64 (futex_word=futex_word@entry=0x7ffff7fb12ac <libtas::unity_condition+44>, expected=expected@entry=0, clockid=clockid@entry=1, abstime=abstime@entry=0x7fffecefce70, private=private@entry=0) at ./nptl/futex-internal.c:139
     * #3  0x00007ffff79c8a3f in __pthread_cond_wait_common (abstime=<optimized out>, clockid=1, mutex=0x7ffff7fb12c0 <libtas::unity_mutex>, cond=0x7ffff7fb1280 <libtas::unity_condition>) at ./nptl/pthread_cond_wait.c:503
     * #4  ___pthread_cond_clockwait64 (abstime=<optimized out>, clockid=1, mutex=0x7ffff7fb12c0 <libtas::unity_mutex>, cond=0x7ffff7fb1280 <libtas::unity_condition>) at ./nptl/pthread_cond_wait.c:682
     * #5  ___pthread_cond_clockwait64 (cond=0x7ffff7fb1280 <libtas::unity_condition>, mutex=0x7ffff7fb12c0 <libtas::unity_mutex>, clockid=1, abstime=<optimized out>) at ./nptl/pthread_cond_wait.c:670
     * #6  0x00007ffff7ea927c in std::__condvar::wait_until (__abs_time=..., __clock=1, __m=..., this=0x7ffff7fb1280 <libtas::unity_condition>) at /usr/include/c++/12/bits/std_mutex.h:169
     * #7  std::condition_variable::__wait_until_impl<std::chrono::duration<long, std::ratio<1l, 1000000000l> > > (__lock=<synthetic pointer>..., __atime=..., this=0x7ffff7fb1280 <libtas::unity_condition>) at /usr/include/c++/12/condition_variable:200
     * #8  std::condition_variable::wait_until<std::chrono::duration<long, std::ratio<1l, 1000000000l> > > (__atime=..., __lock=<synthetic pointer>..., this=0x7ffff7fb1280 <libtas::unity_condition>) at /usr/include/c++/12/condition_variable:110
     * #9  std::condition_variable::wait_for<long, std::ratio<1l, 1000l> > (__rtime=..., __lock=<synthetic pointer>..., this=0x7ffff7fb1280 <libtas::unity_condition>) at /usr/include/c++/12/condition_variable:162
     * #10 libtas::GameHacks::unitySyncWait () at GameHacks.cpp:136
     * #11 0x00007ffff7ed9044 in libtas::sem_wait (sem=<optimized out>) at general/pthreadwrappers.cpp:652
     * ...
     *
     * Thread 3 (that has finished its job and tries to signal the conditional variable)
     * #0  futex_wait (private=0, expected=3, futex_word=0x7ffff7fb1290 <libtas::unity_condition+16>) at ../sysdeps/nptl/futex-internal.h:146
     * #1  futex_wait_simple (private=0, expected=3, futex_word=0x7ffff7fb1290 <libtas::unity_condition+16>) at ../sysdeps/nptl/futex-internal.h:177
     * #2  __condvar_quiesce_and_switch_g1 (private=0, g1index=<synthetic pointer>, wseq=<optimized out>, cond=0x7ffff7fb1280 <libtas::unity_condition>) at ./nptl/pthread_cond_common.c:276
     * #3  ___pthread_cond_broadcast (cond=0x7ffff7fb1280 <libtas::unity_condition>) at ./nptl/pthread_cond_broadcast.c:72
     * #4  0x00007ffff7ea9162 in libtas::GameHacks::unitySyncNotify () at GameHacks.cpp:118
     * #5  0x00007ffff7ed9034 in libtas::sem_wait (sem=0x7ffff58f8c00) at general/pthreadwrappers.cpp:648
     * ...
     */
    
    debuglogstdio(LCF_WAIT, "   Wait that all Unity jobs finish");
    unity_mutex.lock();
    while (unity_waiting_threads || (unity_running_threads > unity_nonterminating_threads)) {
        unity_mutex.unlock();
        NATIVECALL(usleep(100));        
        unity_mutex.lock();
    }
    unity_mutex.unlock();
}

void GameHacks::setCoreclr()
{
    debuglogstdio(LCF_HOOK | LCF_ERROR, "   detected coreclr");
    coreclr = true;
}

bool GameHacks::hasCoreclr()
{
    return coreclr;
}

void GameHacks::setFinalizerThread(pid_t pid)
{
    debuglogstdio(LCF_HOOK | LCF_ERROR, "   set finalizer to %d", pid);
    finalizer_pid = pid;
}

pid_t GameHacks::getFinalizerThread()
{
    return finalizer_pid;
}

}
