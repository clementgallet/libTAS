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

#include "GameHacks.h"
#include "logging.h"
#ifdef __unix__
#include "checkpoint/ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "checkpoint/MachVmMaps.h"
#endif
#include "checkpoint/MemArea.h"

namespace libtas {

bool GameHacks::unity = false;
bool GameHacks::coreclr = false;
pid_t GameHacks::finalizer_pid = 0;

uintptr_t GameHacks::executableBase = 0;
uintptr_t GameHacks::executableEnd = 0;
uintptr_t GameHacks::unityLoadingThreadAddr = 0;
std::ptrdiff_t GameHacks::unityLoadingThreadId = 0;

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

bool GameHacks::isUnityLoadingThread(uintptr_t addr, std::ptrdiff_t routine_id)
{
    if (!unity) return false;
    
    /* The first Unity thread that executes a routine from its executable seems
     * to always be a loading thread */
    
    getExecutableMemory();
    
    if (unityLoadingThreadAddr != 0)
        return (unityLoadingThreadAddr == addr) && (unityLoadingThreadId == routine_id);

    if ((addr >= executableBase) && (addr < executableEnd)) {
        unityLoadingThreadAddr = addr;
        unityLoadingThreadId = routine_id;
        return true;
    }
    
    return false;
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
