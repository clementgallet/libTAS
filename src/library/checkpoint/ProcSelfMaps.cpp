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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "ProcSelfMaps.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../Utils.h"
#include <cstring>
#include "ReservedMemory.h"

namespace libtas {

ProcSelfMaps::ProcSelfMaps() : off(0)
{
    /* We need to copy /proc/self/maps, because it can be modified while parsing it */
    int fd;
    NATIVECALL(fd = open("/proc/self/maps", O_RDONLY));
    MYASSERT(fd != -1);
    NATIVECALL(tmp_fd = open("/tmp", O_RDWR | O_TMPFILE));
    MYASSERT(tmp_fd != -1);
    
    ssize_t sz = 1;
    
    while (sz > 0) {
        char buf[4096];
        sz = Utils::readAll(fd, buf, 4096);
        Utils::writeAll(tmp_fd, buf, sz);
    }
    NATIVECALL(close(fd));
}

ProcSelfMaps::~ProcSelfMaps()
{
    NATIVECALL(close(tmp_fd));
}

void ProcSelfMaps::reset()
{
    off = 0;
}

uintptr_t ProcSelfMaps::readDec()
{
    uintptr_t v = 0;

    while (1) {
        char c = line[line_idx];
        if ((c >= '0') && (c <= '9')) {
            c -= '0';
        } else {
            break;
        }
        v = v * 10 + c;
        line_idx++;
    }
    return v;
}

uintptr_t ProcSelfMaps::readHex()
{
    uintptr_t v = 0;

    while (1) {
        char c = line[line_idx];
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
        line_idx++;
    }
    return v;
}

bool ProcSelfMaps::getNextArea(Area *area)
{
    ssize_t ret = pread(tmp_fd, line, 1024, off);
    if (ret < 1) {
        area->addr = nullptr;
        area->size = 0;
        return false;        
    }
    line_idx = 0;

    uintptr_t addr = readHex();
    area->addr = reinterpret_cast<void*>(addr);

    MYASSERT(line[line_idx++] == '-')

    uintptr_t endAddr = readHex();
    MYASSERT(endAddr != 0)
    area->endAddr = reinterpret_cast<void*>(endAddr);

    MYASSERT(line[line_idx++] == ' ')

    MYASSERT(endAddr >= addr)
    area->size = static_cast<size_t>(endAddr - addr);

    char rflag = line[line_idx++];
    MYASSERT((rflag == 'r') || (rflag == '-'))

    char wflag = line[line_idx++];
    MYASSERT((wflag == 'w') || (wflag == '-'))

    char xflag = line[line_idx++];
    MYASSERT((xflag == 'x') || (xflag == '-'))

    char sflag = line[line_idx++];
    MYASSERT((sflag == 's') || (sflag == 'p'))

    MYASSERT(line[line_idx++] == ' ')

    area->offset = readHex();
    MYASSERT(line[line_idx++] == ' ')

    area->devmajor = readHex();
    MYASSERT(line[line_idx++] == ':')

    area->devminor = readHex();
    MYASSERT(line[line_idx++] == ' ')

    area->inodenum = readDec();

    while (line[line_idx] == ' ') {
        line_idx++;
    }

    area->name[0] = '\0';
    if (line[line_idx] == '/' || line[line_idx] == '[' || line[line_idx] == '(') {
        // absolute pathname, or [stack], [vdso], etc.
        size_t i = 0;
        while (line[line_idx] != '\n') {
            area->name[i++] = line[line_idx++];
            MYASSERT(i < sizeof(area->name))
        }
        area->name[i] = '\0';
    }

    MYASSERT(line[line_idx++] == '\n')
    off += line_idx;

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

    /* Max protection does not exist on Linux, so setting all flags */
    area->max_prot = PROT_READ | PROT_WRITE | PROT_EXEC;

    if (sflag == 's') {
        area->flags = Area::AREA_SHARED;
    }
    if (sflag == 'p') {
        area->flags = Area::AREA_PRIV;
    }
    if (area->name[0] == '\0') {
        area->flags |= Area::AREA_ANON;
    }
    if (area->name[0] == '/') {
        area->flags |= Area::AREA_FILE;
    }

    area->skip = false;

    /* Identify specific segments */
    if (strstr(area->name, "[stack"))
        area->flags |= Area::AREA_STACK;

    if (strcmp(area->name, "[heap]") == 0)
        area->flags |= Area::AREA_HEAP;

    /* Sometimes the [heap] is split into several contiguous segments, such as
     * after a dumping was made (but why...?). This can screw up our code for
     * loading and remapping the [heap] using brk, so we always read the [heap]
     * as one single segment.
     */
    if (area->flags & Area::AREA_HEAP) {
        Area next_area;
        off_t cur_off = off;
        bool valid = getNextArea(&next_area); // recursive call
        if (valid && (next_area.flags & Area::AREA_HEAP)) {
            MYASSERT(area->endAddr == next_area.addr)
            MYASSERT(area->flags == next_area.flags)
            area->prot |= next_area.prot;
            area->endAddr = next_area.endAddr;
            area->size += next_area.size;
        }
        else {
            off = cur_off;
        }
    }

    return true;
}

}
