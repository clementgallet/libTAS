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
 */

#include "kernel32.h"
#include "winehook.h"
#include "../hookpatch.h"
#include "../logging.h"
#include "../DeterministicTimer.h"
#include "../../shared/SharedConfig.h"

#include <stdint.h>
#include <inttypes.h>

namespace libtas {

typedef union _LARGE_INTEGER {
    struct {
        unsigned int LowPart;
        int HighPart;
    } dummy;
    struct {
        unsigned int LowPart;
        int HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER;

namespace orig {

static int __stdcall __attribute__((noinline)) WaitForMultipleObjectsEx( int count, const void **handles,
                                           bool wait_all, int timeout,
                                           bool alertable )
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static unsigned int __stdcall __attribute__((noinline)) GetTickCount()
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static uint64_t __stdcall __attribute__((noinline)) GetTickCount64()
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static int __stdcall __attribute__((noinline)) QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static int __stdcall __attribute__((noinline)) QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

}

int __stdcall WaitForMultipleObjectsEx( int count, const void **handles,
                                       bool wait_all, int timeout,
                                       bool alertable )
{
    DEBUGLOGCALL(LCF_WINE);
    return orig::WaitForMultipleObjectsEx(count, handles, wait_all, timeout, alertable);
}

unsigned int __stdcall GetTickCount()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_GETTICKCOUNT);
    unsigned int msec = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %d", msec);
    return msec;
}

uint64_t __stdcall GetTickCount64()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_GETTICKCOUNT64);
    uint64_t msec = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %" PRIu64, msec);
    return msec;
}

int __stdcall QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    DEBUGLOGCALL(LCF_TIMEGET);
    lpFrequency->QuadPart = 1000000000;
    return 1;
}

int __stdcall QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER);
    lpPerformanceCount->QuadPart = ts.tv_nsec + ts.tv_sec * 1000000000LL;
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %" PRId64, lpPerformanceCount->QuadPart);
    return 1;
}

void hook_kernel32()
{
    // HOOK_PATCH_ORIG(WaitForMultipleObjectsEx, "kernel32.dll.so");
    HOOK_PATCH_ORIG(GetTickCount, "kernel32.dll.so");
    HOOK_PATCH_ORIG(GetTickCount64, "kernel32.dll.so");
    HOOK_PATCH_ORIG(QueryPerformanceFrequency, "kernel32.dll.so");
    HOOK_PATCH_ORIG(QueryPerformanceCounter, "kernel32.dll.so");
}


}
