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

#include "BusyLoopDetection.h"
// #include <sstream>
#include "logging.h"
#include "frame.h"
#include "backtrace.h"
#include "DeterministicTimer.h"
//#include "../shared/SharedConfig.h"
// #include "global.h" // game_info
// #include <math.h>
#include <execinfo.h>
#include <map>
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include <dlfcn.h>
#include <sstream>
#include <string.h>
#include <stdint.h>
#include "GlobalState.h"
#include "checkpoint/ProcSelfMaps.h"
#include "checkpoint/ProcMapsArea.h"
#include "../shared/SharedConfig.h"

extern char**environ;

#define MAX_STACK_SIZE 256

namespace libtas {

static uint64_t hash;
static uint64_t timecall_count;

void BusyLoopDetection::reset()
{
    if (!shared_config.busyloop_detection)
        return;

    /* Remove any fake ticks cause by the busy loop detector */
    detTimer.fakeAdvanceTimer({0, 0});

    timecall_count = 0;
}

void BusyLoopDetection::resetHash()
{
    hash = 0;
}

void BusyLoopDetection::toHash(const char* string)
{
    const char* c = string;
    for (; *c != '\0'; c++)
        hash = hash * 33 + *c;
}

void BusyLoopDetection::toHash(intptr_t addr)
{
    hash = hash * 33 + addr;
}

void BusyLoopDetection::increment(int type)
{
    if (!shared_config.busyloop_detection && !shared_config.time_trace)
        return;
    if (!ThreadManager::isMainThread())
        return;
    if (GlobalState::isNative())
        return;
    if (detTimer.isInsideFrameBoundary())
        return;

    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "Time function called");

    GlobalState::setNative(true);

    resetHash();

    toHash(static_cast<intptr_t>(type));

    void* addresses[MAX_STACK_SIZE];
    const int n = backtrace(addresses, MAX_STACK_SIZE);

    /* Get the ld_library_path content */
    /* The env name was modified in libTAS init function */
    static char* ld_path = nullptr;

    if (ld_path == nullptr) {
        const char* ld = "DD_LIBRARY_PATH=";
        for (int i=0; environ[i]; i++) {
            if (strstr(environ[i], ld) == environ[i]) {
                ld_path = environ[i] + strlen(ld);
                /* Check if non empty */
                if (ld_path[0] == '\0')
                    ld_path = nullptr;
                break;
            }
        }
    }

    /* We don't need the whole `backtrace_symbols()` feature, only some information,
     * so this is a simplified implementation of this function. */
    std::ostringstream oss;

    /* Start the stack at frame 3 to skip this, DeterministicTimer::getTicks() and gettime() */
    for (int cnt = 3; cnt < n; ++cnt) {
        Dl_info info;
        int status = dladdr(addresses[cnt], &info);
        if (status && info.dli_fname != NULL && info.dli_fname[0] != '\0') {
            /* Check if the program or library is provided by the game,
             * using the content of LD_LIBRARY_PATH
             */
            bool isGameLibrary = false;
            /* Putting executable base addresses directly, because I'm lazy... */
            if (info.dli_fbase == (void*)0x400000 || info.dli_fbase == (void*)0x8048000)
                isGameLibrary = true;
            else if (ld_path) {
                isGameLibrary = strstr(info.dli_fname, ld_path);
            }

            if (isGameLibrary) {
                /* Hash the file name */
                const char* filename = strrchr(info.dli_fname, '/');
                toHash(filename? ++filename : info.dli_fname);

                /* Hash the address offset */
                if (info.dli_fbase && (addresses[cnt] >= info.dli_fbase))
                    toHash(reinterpret_cast<intptr_t>(addresses[cnt]) - reinterpret_cast<intptr_t>(info.dli_fbase));
            }
            else {
                /* We should be safe to push the function called inside the library.
                 * everything else may change (even library name) */
                if (info.dli_sname != NULL) {
                    toHash(info.dli_sname);
                }
            }

            /* Building stack trace string */
            if (shared_config.time_trace) {
                oss << info.dli_fname;

                if (info.dli_sname == NULL)
                    info.dli_saddr = info.dli_fbase;

                if (info.dli_sname != NULL || info.dli_saddr != 0) {
                    oss << "(" << (info.dli_sname ? info.dli_sname : "");
                    if (info.dli_saddr != 0) {
                        if (addresses[cnt] >= (void *)info.dli_saddr) {
                            oss << '+' << std::hex << (reinterpret_cast<intptr_t>(addresses[cnt]) - reinterpret_cast<intptr_t>(info.dli_saddr));
                        }
                        else {
                            oss << '-' << std::hex << (reinterpret_cast<intptr_t>(info.dli_saddr) - reinterpret_cast<intptr_t>(addresses[cnt]));
                        }
                    }
                    oss << ")";
                }
                oss << " ";
            }
        }
        else {
            /* Executed code comes from some anonymous mapping, which is often
             * the sign of JIT execution. For now, we trust that the code always
             * has the same offset from the beginning of the mapped section. */

            /* Find the corresponding memory area */
            ProcSelfMaps procSelfMaps;
            Area area;
            while (procSelfMaps.getNextArea(&area)) {
                if ((addresses[cnt] >= area.addr) && (addresses[cnt] < area.endAddr)) {
                    toHash(reinterpret_cast<intptr_t>(addresses[cnt]) - reinterpret_cast<intptr_t>(area.addr));
                    break;
                }
            }
        }
        if (shared_config.time_trace) {
            oss << "[" << addresses[cnt] << "]\n";
        }
    }

    if (shared_config.time_trace) {
        lockSocket();
        sendMessage(MSGB_GETTIME_BACKTRACE);
        sendData(&type, sizeof(int));
        sendData(&hash, sizeof(uint64_t));
        sendString(oss.str());
        unlockSocket();
    }
    GlobalState::setNative(false);

    if (hash == shared_config.busy_loop_hash) {
        timecall_count++;

        if (timecall_count == 10) {
            debuglogstdio(LCF_TIMESET, "Busy loop detected, fake advance ticks to next frame");
            detTimer.fakeAdvanceTimerFrame();
        }
        if (timecall_count > 20) {
            debuglogstdio(LCF_TIMESET, "Still softlocking, advance one frame");
            std::function<void()> dummy_draw;
#ifdef LIBTAS_ENABLE_HUD
            static RenderHUD dummy;
            frameBoundary(dummy_draw, dummy);
#else
            frameBoundary(dummy_draw);
#endif
        }
    }
}

}
