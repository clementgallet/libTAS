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

#ifndef LIBTAS_GAMEHACKS_H_INCLUDED
#define LIBTAS_GAMEHACKS_H_INCLUDED

#include <sys/types.h>
#include <cstddef>
#include <cstdint>

namespace libtas {

namespace GameHacks
{
void setUnity();

bool isUnity();
        
void getExecutableMemory();

bool isUnityLoadingThread(uintptr_t addr);

/* Synchronize methods so that Unity jobs are running sequentially. It also
 * supports jobs that never finish by using a timeout */
void unitySyncNotify();
void unitySyncWait();
void unitySyncWaitAll();

/* Regsiter that the game linked `libcoreclr.so` library */
void setCoreclr();
    
/* Returns if the game linked `libcoreclr.so` library */
bool hasCoreclr();

/* Regsiter the pid of the .NET finalizer thread */
void setFinalizerThread(pid_t pid);

/* Get the pid of the .NET finalizer thread, or 0 */
pid_t getFinalizerThread();        
        
};

}

#endif
