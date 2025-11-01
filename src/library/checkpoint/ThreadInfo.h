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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */

#ifndef LIBTAS_THREAD_INFO_H
#define LIBTAS_THREAD_INFO_H

#include "ThreadLocalStorage.h"
#include "Profiler.h"

#include <atomic>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#include <ucontext.h>
#include <pthread.h> // pthread_t
#include <string>

namespace libtas {

struct ThreadInfo {
    enum ThreadState {
        ST_UNINITIALIZED, // thread was just created
        ST_RUNNING, // thread is running normally
        ST_SIGNALED, // a suspend signal has been sent to the thread
        ST_SUSPINPROG, // thread is in the middle of being suspended
        ST_SUSPENDED, // thread is suspended
        ST_ZOMBIE, // thread has returned but is not yet joined or detached
        ST_CKPNTHREAD, // thread that does the checkpoint
        ST_TERMINATING, // thread flagged as needed to be terminated
        ST_TERMINATED, // thread that will terminate
    };

    ThreadState state = ST_UNINITIALIZED; // thread state
    ThreadState orig_state = ST_UNINITIALIZED; // thread state before savestate
    pthread_t pthread_id = 0; // tid of the thread
    pid_t real_tid = 0; // real tid of the thread
    pid_t translated_tid = 0; // translated tid of the thread, as seen by the game
    pid_t* ptid = nullptr; // pointer in pthread_t struct to the tid
    void *(*start)(void *) = nullptr; // original start function of the thread
    void *arg = nullptr; // original argument of the start function
    bool detached = false; // flag to keep track if the thread was detached
    std::ptrdiff_t routine_id; // mostly unique identifier of a start function,
                               // constant across instances of the game
    ucontext_t savctx; // context of the thread (registers)
    void* saved_sp; // stack pointer of suspended thread
    ThreadTLSInfo tlsInfo; // thread local storage information not stored in
                           // memory (registers)
    void* retval; // return value of the original start function
    bool initial_native = false; // initial value of the global native state
    bool initial_owncode = false; // initial value of the global owncode state
    bool initial_nolog = false; // initial value of the global nolog state

    std::string name; // name of the thread
    
    void* stack_addr = nullptr; // stack address of thread
    size_t stack_size = 0; // stack size of thread

    bool syncEnabled = false; // main thread needs to wait for this thread
    bool syncGo = false; // main thread can advance for now
    std::atomic<int> syncCount;
    int syncOldCount = 0;

    bool unityThread = false; // is unity wait thread
    int unityJobCount = 0; // job count executed by this thread during the current frame

    Profiler::Database* profilerDatabase;

    ThreadInfo *next = nullptr; // next thread info in the linked list
    ThreadInfo *prev = nullptr; // previous thread info in the linked list
};
}

#endif
