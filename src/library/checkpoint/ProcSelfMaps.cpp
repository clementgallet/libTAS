/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ProcSelfMaps.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../Utils.h"
#include <cstring>

namespace libtas {

ProcSelfMaps::ProcSelfMaps(void* restoreAddr, size_t restoreLength)
    : dataIdx(0),
    numAreas(0),
    numBytes(0)
{
    int fd = open("/proc/self/maps", O_RDONLY);
    MYASSERT(fd != -1);

    data = static_cast<char*>(restoreAddr);

    numBytes = Utils::readAll(fd, data, restoreLength);
    MYASSERT(numBytes > 0)
    MYASSERT(numBytes < restoreLength)

    close(fd);

    for (size_t i = 0; i < numBytes; i++) {
        if (data[i] == '\n') {
            numAreas++;
        }
    }
}

void ProcSelfMaps::reset()
{
    dataIdx = 0;
}

uintptr_t ProcSelfMaps::readDec()
{
    uintptr_t v = 0;

    while (1) {
        char c = data[dataIdx];
        if ((c >= '0') && (c <= '9')) {
            c -= '0';
        } else {
            break;
        }
        v = v * 10 + c;
        dataIdx++;
    }
    return v;
}

uintptr_t ProcSelfMaps::readHex()
{
    uintptr_t v = 0;

    while (1) {
        char c = data[dataIdx];
        if ((c >= '0') && (c <= '9')) {
            c -= '0';
        } else if ((c >= 'a') && (c <= 'f')) {
            c -= 'a' - 10;
        } else if ((c >= 'A') && (c <= 'F')) {
            c -= 'A' - 10;
        } else {
            break;
        }
        v = v * 16 + c;
        dataIdx++;
    }
    return v;
}

bool ProcSelfMaps::getNextArea(Area *area)
{
    if (dataIdx >= numBytes || data[dataIdx] == 0) {
        area->addr = nullptr;
        area->size = 0;
        return false;
    }

    uintptr_t addr = readHex();
    MYASSERT(addr != 0)
    area->addr = reinterpret_cast<void*>(addr);

    MYASSERT(data[dataIdx++] == '-')

    uintptr_t endAddr = readHex();
    MYASSERT(endAddr != 0)
    area->endAddr = reinterpret_cast<void*>(endAddr);

    MYASSERT(data[dataIdx++] == ' ')

    MYASSERT(endAddr >= addr)
    area->size = static_cast<size_t>(endAddr - addr);

    char rflag = data[dataIdx++];
    MYASSERT((rflag == 'r') || (rflag == '-'))

    char wflag = data[dataIdx++];
    MYASSERT((wflag == 'w') || (wflag == '-'))

    char xflag = data[dataIdx++];
    MYASSERT((xflag == 'x') || (xflag == '-'))

    char sflag = data[dataIdx++];
    MYASSERT((sflag == 's') || (sflag == 'p'))

    MYASSERT(data[dataIdx++] == ' ')

    area->offset = readHex();
    MYASSERT(data[dataIdx++] == ' ')

    area->devmajor = readHex();
    MYASSERT(data[dataIdx++] == ':')

    area->devminor = readHex();
    MYASSERT(data[dataIdx++] == ' ')

    area->inodenum = readDec();

    while (data[dataIdx] == ' ') {
        dataIdx++;
    }

    area->name[0] = '\0';
    if (data[dataIdx] == '/' || data[dataIdx] == '[' || data[dataIdx] == '(') {
        // absolute pathname, or [stack], [vdso], etc.
        size_t i = 0;
        while (data[dataIdx] != '\n') {
            area->name[i++] = data[dataIdx++];
            MYASSERT(i < sizeof(area->name))
        }
        area->name[i] = '\0';
    }

    MYASSERT(data[dataIdx++] == '\n')

    area->prot = 0;
    if (rflag == 'r') {
        area->prot |= PROT_READ;
    }
    if (wflag == 'w') {
        area->prot |= PROT_WRITE;
    }
    if (xflag == 'x') {
        area->prot |= PROT_EXEC;
    }

    area->flags = MAP_FIXED;
    if (sflag == 's') {
        area->flags |= MAP_SHARED;
    }
    if (sflag == 'p') {
        area->flags |= MAP_PRIVATE;
    }
    if (area->name[0] == '\0') {
        area->flags |= MAP_ANONYMOUS;
    }

    area->skip = false;

    /* Sometimes the [heap] is split into several contiguous segments, such as
     * after a dumping was made (but why...?). This can screw up our code for
     * loading and remapping the [heap] using brk, so we always read the [heap]
     * as one single segment.
     */
    if (strcmp(area->name, "[heap]") == 0) {
        Area next_area;
        size_t curDataIdx = dataIdx;
        bool valid = getNextArea(&next_area); // recursive call
        if (valid && (strcmp(next_area.name, "[heap]") == 0)) {
            MYASSERT(area->endAddr == next_area.addr)
            MYASSERT(area->prot == next_area.prot)
            MYASSERT(area->flags == next_area.flags)
            area->endAddr = next_area.endAddr;
            area->size += next_area.size;
        }
        else {
            dataIdx = curDataIdx;
        }
    }

    return true;
}

}
