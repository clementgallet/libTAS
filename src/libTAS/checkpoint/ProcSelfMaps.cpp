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

#include "ProcSelfMaps.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "Utils.h"

ProcSelfMaps::ProcSelfMaps()
    : dataIdx(0),
    numAreas(0),
    numBytes(0),
    fd(-1)
{
    char buf[4096];

    fd = open("/proc/self/maps", O_RDONLY);
    MYASSERT(fd != -1);
    ssize_t numRead = 0;

    // Get an approximation of the required buffer size.
    do {
        numRead = Utils::readAll(fd, buf, sizeof(buf));
        if (numRead > 0) {
            numBytes += numRead;
        }
    } while (numRead > 0);

    // Now allocate a buffer. Note that this will most likely change the layout
    // of /proc/self/maps, so we need to recalculate numBytes.
    size_t size = numBytes + 4096; // Add a one page buffer.
    data = (char *)malloc(size);
    MYASSERT(lseek(fd, 0, SEEK_SET) == 0)

    numBytes = Utils::readAll(fd, data, size);
    MYASSERT(numBytes > 0)

    close(fd);

    for (size_t i = 0; i < numBytes; i++) {
        if (data[i] == '\n') {
            numAreas++;
        }
    }
}

ProcSelfMaps::~ProcSelfMaps()
{
    free(data);
    fd = -1;
    dataIdx = 0;
    numAreas = 0;
    numBytes = 0;
}

unsigned long int ProcSelfMaps::readDec()
{
    unsigned long int v = 0;

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

unsigned long int ProcSelfMaps::readHex()
{
    unsigned long int v = 0;

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

int ProcSelfMaps::getNextArea(Area *area)
{
    char rflag, sflag, wflag, xflag;

    if (dataIdx >= numBytes || data[dataIdx] == 0) {
        return 0;
    }

    area->addr = (VA)readHex();
    MYASSERT(area->addr != NULL)

    MYASSERT(data[dataIdx++] == '-')

    area->endAddr = (VA)readHex();
    MYASSERT(area->endAddr != NULL)

    MYASSERT(data[dataIdx++] == ' ')

    MYASSERT(area->endAddr >= area->addr)
    area->size = area->endAddr - area->addr;

    rflag = data[dataIdx++];
    MYASSERT((rflag == 'r') || (rflag == '-'))

    wflag = data[dataIdx++];
    MYASSERT((wflag == 'w') || (wflag == '-'))

    xflag = data[dataIdx++];
    MYASSERT((xflag == 'x') || (xflag == '-'))

    sflag = data[dataIdx++];
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

    area->properties = 0;

    return 1;
}
