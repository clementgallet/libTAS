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

#include "monowrappers.h"
#include "sleepwrappers.h" // transfer_sleep()

#include "logging.h"
#include "hookpatch.h"
#include "global.h"
#include "DeterministicTimer.h"
#include "GlobalState.h"
#include "checkpoint/ThreadManager.h" // isMainThread()

namespace libtas {

namespace orig {

unsigned int (*ves_icall_System_Threading_Thread_Sleep_internal)(int ms, void *error);
uint8_t (*ves_icall_System_Threading_ThreadPool_SetMinThreadsNative)(int32_t worker_threads, int32_t completion_port_threads, void *error);
uint8_t (*ves_icall_System_Threading_ThreadPool_SetMaxThreadsNative)(int32_t worker_threads, int32_t completion_port_threads, void *error);
void (*ves_icall_System_Threading_ThreadPool_GetMinThreadsNative)(int32_t *worker_threads, int32_t *completion_port_threads, void *error);
void (*ves_icall_System_Threading_ThreadPool_GetMaxThreadsNative)(int32_t *worker_threads, int32_t *completion_port_threads, void *error);
uint8_t (*ves_icall_System_Threading_ThreadPool_RequestWorkerThread)(void *error);

}

void ves_icall_System_Threading_Thread_Sleep_internal(int ms, void *error)
{
        
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    if (GlobalState::isNative()) {
        orig::ves_icall_System_Threading_Thread_Sleep_internal(ms, error);
        return;
    }

    if (ms == 0)
        return;

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d ms", __func__, ms);

    if (! transfer_sleep(ts))
        orig::ves_icall_System_Threading_Thread_Sleep_internal(ms, error);
}

void ves_icall_System_Threading_ThreadPool_GetMinThreadsNative (int32_t *worker_threads, int32_t *completion_port_threads, void *error)
{
}

uint8_t ves_icall_System_Threading_ThreadPool_SetMinThreadsNative (int32_t worker_threads, int32_t completion_port_threads, void *error)
{
    debuglogstdio(LCF_THREAD, "%s call with worker_threads %d and completion_port_threads %d", __func__, worker_threads, completion_port_threads);
    return 1;
}

uint8_t ves_icall_System_Threading_ThreadPool_SetMaxThreadsNative (int32_t worker_threads, int32_t completion_port_threads, void *error)
{
    debuglogstdio(LCF_THREAD, "%s call with worker_threads %d and completion_port_threads %d", __func__, worker_threads, completion_port_threads);
    return 1;
}

uint8_t ves_icall_System_Threading_ThreadPool_RequestWorkerThread (void *error)
{
    debuglogstdio(LCF_THREAD, "%s call", __func__);
    
    static bool threadInit = false;
    if (!threadInit) {
        int32_t worker_threads = 0, completion_port_threads = 0;
        orig::ves_icall_System_Threading_ThreadPool_GetMinThreadsNative (&worker_threads, &completion_port_threads, nullptr);
        orig::ves_icall_System_Threading_ThreadPool_SetMaxThreadsNative(worker_threads, completion_port_threads, nullptr);
        
        threadInit = true;
    }
    
    return orig::ves_icall_System_Threading_ThreadPool_RequestWorkerThread(error);
}

void hook_mono()
{
    HOOK_PATCH_ORIG(ves_icall_System_Threading_Thread_Sleep_internal, nullptr);
    HOOK_PATCH_ORIG(ves_icall_System_Threading_ThreadPool_SetMinThreadsNative, nullptr);
    HOOK_PATCH_ORIG(ves_icall_System_Threading_ThreadPool_SetMaxThreadsNative, nullptr);
    HOOK_PATCH_ORIG(ves_icall_System_Threading_ThreadPool_GetMinThreadsNative, nullptr);
    HOOK_PATCH_ORIG(ves_icall_System_Threading_ThreadPool_RequestWorkerThread, nullptr);
    
}

}
