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
 */

#ifndef LIBTAS_UNITYHACKS_H_INCLUDED
#define LIBTAS_UNITYHACKS_H_INCLUDED

#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace libtas {

namespace UnityHacks
{

// From ImPlot
struct ScrollingBuffer {
    size_t MaxSize;
    int Offset;
    std::vector<float> DataX;
    std::vector<float> DataY;
    std::string name;

    ScrollingBuffer(int max_size);
    void AddPoint(int x, int y);
    void Erase();
};

struct ScrollingBuffers {
    std::map<int, ScrollingBuffer> Buffers;
    
    ScrollingBuffers();
    void AddPoint(float x, float y, int tid);
};

const ScrollingBuffers& getJobData();
    
void setUnity();

bool isUnity();
        
void getExecutableMemory();

bool isLoadingThread(uintptr_t addr);

/* Synchronize methods so that Unity jobs are running sequentially. It also
 * supports jobs that never finish by using a timeout */
void syncNotify();
void syncWait();
void syncWaitAll();

/* Determine if a given thread is a Unity loading thread from their thread name */
void waitFromName(pthread_t target_thread, const char *name);

/* Patch Unity function given the function address */
void patch(uint64_t addr);

};

}

#endif
