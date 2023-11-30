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

#include "systemwrappers.h"

#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#include "backtrace.h"
#ifdef __unix__
#include "xlib/xdisplay.h" // x11::gameDisplays
#endif

#include <execinfo.h>
#include <string.h>
#include <signal.h>

namespace libtas {

DEFINE_ORIG_POINTER(getpid)
DEFINE_ORIG_POINTER(fork)
DEFINE_ORIG_POINTER(sched_getaffinity)
DEFINE_ORIG_POINTER(__sched_cpucount)

/* Override */ pid_t getpid (void) __THROW
{
    LINK_NAMESPACE_GLOBAL(getpid);
    pid_t pid = orig::getpid();
    
    if (GlobalState::isNative()) {
        return pid;
    }

    DEBUGLOGCALL(LCF_SYSTEM);

    /* A bit hackish: Dead Cells seeds the rng using the process pid.
     * However, specifying a wrong pid for other calls will most likely crash
     * games (mainly because OpenGL drivers need this value). So we look from
     * which library was the call made. */
    void* return_address =  __builtin_return_address(0);
    char** symbols = backtrace_symbols(&return_address, 1);
    if (symbols != nullptr) {
        if (strstr(symbols[0], "libhl.so") || strstr(symbols[0], "PapersPlease(+0x"))
            pid = 1234;
        free(symbols);
    }

    return pid;
}

/* Override */  pid_t fork(void) __THROWNL
{
    LINK_NAMESPACE_GLOBAL(fork);
    pid_t pid = orig::fork();

    if (GlobalState::isNative()) {
        return pid;
    }

    DEBUGLOGCALL(LCF_SYSTEM);

    if (pid == 0) {
        Global::is_fork = true;

#ifdef __unix__
        /* Invalidate the previous connections to the X server */
        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            x11::gameDisplays[i] = nullptr;
        }
#endif
        
        /* We are not interested in the forked process, make it run native.
         * Fix one Unity game (Stephen Sausage Roll), where forked process
         * closes a pipe handle and can softlock on mutex. */
        GlobalState::setNative(true);
    }
    return pid;
}

/* Override */ int __sched_cpucount (size_t setsize, const cpu_set_t *setp) __THROW
{
    LINK_NAMESPACE_GLOBAL(__sched_cpucount);

    /* Check if called from GPU driver. If so, return the real value */
    void* return_address =  __builtin_return_address(0);
    char** symbols = backtrace_symbols(&return_address, 1);
    if (symbols != nullptr) {
        if (strstr(symbols[0], "nouveau_dri.so") || strstr(symbols[0], "swrast_dri.so")) {
            // debuglogstdio(LCF_TIMEGET | LCF_FREQUENT | LCF_ERROR, "  special workaround");
            int ret = orig::__sched_cpucount (setsize, setp);
            free(symbols);
            return ret;
        }
        free(symbols);
    }

    // raise(SIGSEGV);
    DEBUGLOGCALL(LCF_SYSTEM);
    return 1;
}

}
