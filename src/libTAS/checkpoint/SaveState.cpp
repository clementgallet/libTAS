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

#include "SaveState.h"
#include "Utils.h"
#include "StateHeader.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
// #include <sys/mman.h>
// #include <cstring>

namespace libtas {

SaveState::SaveState(char* pagemappath, char* pagespath)
{
    if (pagemappath[0] == '\0') {
        pmfd = -1;
        return;
    }

    NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
    MYASSERT(pmfd != -1)

    NATIVECALL(pfd = open(pagespath, O_RDONLY));
    MYASSERT(pfd != -1)

    /* Seek after the savestate header */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);

    /* Read the first area */
    Utils::readAll(pmfd, &area, sizeof(Area));
    current_addr = static_cast<char*>(area.addr);
}

SaveState::~SaveState()
{
    if (pmfd > 0) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }
}

char SaveState::getPageFlag(char* addr)
{
    while ((area.addr != nullptr) && (addr >= static_cast<char*>(area.endAddr))) {
        /* Skip areas until the one we are interested in */
        if (!area.skip) {
            lseek(pmfd, static_cast<intptr_t>(static_cast<char*>(area.endAddr) - current_addr) / 4096, SEEK_CUR);
        }

        Utils::readAll(pmfd, &area, sizeof(Area));
        lseek(pfd, area.page_offset, SEEK_SET);
        current_addr = static_cast<char*>(area.addr);
    }

    // debuglogstdio(LCF_CHECKPOINT, "Savestate addr query %p, current area %p and size %d, with current addr %p", addr, area.addr, area.size, current_addr);

    if (area.addr == nullptr)
        return Area::NONE;

    if (addr < static_cast<char*>(area.addr))
        return Area::NONE;

    if (area.skip)
        return Area::NONE;

    char flag;
    for (; current_addr < addr; current_addr += 4096) {
        Utils::readAll(pmfd, &flag, sizeof(char));
        if (flag == Area::FULL_PAGE) {
            lseek(pfd, 4096, SEEK_CUR);
        }
    }

    Utils::readAll(pmfd, &flag, sizeof(char));
    current_addr += 4096;
    return flag;
}

char* SaveState::getPage(char flag)
{
    if (flag == Area::FULL_PAGE) {
        /* Store the content of the page in this object */
        Utils::readAll(pfd, page, 4096);
    }
    return page;
}

int SaveState::getPageFd()
{
    return pfd;
}

void SaveState::skipPage(char flag)
{
    if (flag == Area::FULL_PAGE) {
        lseek(pfd, 4096, SEEK_CUR);
    }
}

}
