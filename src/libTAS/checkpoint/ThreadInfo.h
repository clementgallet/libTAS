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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */

#ifndef LIBTAS_THREAD_INFO_H
#define LIBTAS_THREAD_INFO_H
#include <atomic>
#include <ucontext.h>
#include <pthread.h> // pthread_t
#include "ThreadLocalStorage.h"

namespace libtas {

struct ThreadInfo {
    enum ThreadState {
        ST_RUNNING, // thread is running normally
        ST_SIGNALED, // a suspend signal has been sent to the thread
        ST_SUSPINPROG, // thread is in the middle of being suspended
        ST_SUSPENDED, // thread is suspended
        ST_ZOMBIE, // thread has returned but is not yet joined or detached
        ST_FAKEZOMBIE, // like ST_ZOMBIE except the actual thread no longer
                       // exists. The thread has been joined by us during a
                       // savestate, so we keep information including the
                       // return value until the game does a join or detach.
        ST_CKPNTHREAD // thread that does the checkpoint
    };

    ThreadState state; // thread state
    pthread_t tid; // tid of the thread
    void *(*start)(void *); // original start function of the thread
    void *arg; // original argument of the start function
    //std::atomic<bool> go;
    bool detached; // flag to keep track if the thread was detached
    std::ptrdiff_t routine_id; // mostly unique identifier of a start function,
                               // constant across instances of the game
    ucontext_t savctx; // context of the thread (registers)
    ThreadTLSInfo tlsInfo; // thread local storage information not stored in
                           // memory (registers)
    void* retval; // return value of the original start function
    bool initial_native; // initial value of the global native state
    bool initial_owncode; // initial value of the global owncode state
    bool initial_nolog; // initial value of the global nolog state

    ThreadInfo *next; // next thread info in the linked list
    ThreadInfo *prev; // previous thread info in the linked list
};
}

#endif
