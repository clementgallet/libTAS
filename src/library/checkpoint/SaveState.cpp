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

#include "SaveState.h"
#include "../Utils.h"
#include "StateHeader.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>

namespace libtas {

SaveState::SaveState(const char* pagemappath, const char* pagespath, int pagemapfd, int pagesfd)
{
    queued_size = 0;

    if (shared_config.savestates_in_ram) {
        pmfd = pagemapfd;
        pfd = pagesfd;
        if (!pmfd) {
            pmfd = -1;
            return;
        }

        lseek(pmfd, 0, SEEK_SET);
        lseek(pfd, 0, SEEK_SET);
    }
    else {
        if (pagemappath[0] == '\0') {
            pmfd = -1;
            return;
        }

        NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
        MYASSERT(pmfd != -1)

        NATIVECALL(pfd = open(pagespath, O_RDONLY));
        MYASSERT(pfd != -1)
    }

    restart();
}

SaveState::~SaveState()
{
    if (!shared_config.savestates_in_ram && (pmfd > 0)) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }
}

void SaveState::readHeader(StateHeader& sh)
{
    lseek(pmfd, 0, SEEK_SET);
    Utils::readAll(pmfd, &sh, sizeof(sh));

    restart();
}

void SaveState::restart()
{
    /* Seek after the savestate header */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);
    flags_remaining = 0;

    /* Read the first area */
    nextArea();
}

char SaveState::nextFlag()
{
    if (flag_i == 4096) {
    	MYASSERT(flags_remaining > 0);

    	int size = (flags_remaining > 4096 ? 4096 : flags_remaining);

    	Utils::readAll(pmfd, flags, size);
    	flags_remaining -= size;

    	flag_i = 0;
    }

    current_flag = flags[flag_i++];
    return current_flag;
}

void SaveState::nextArea()
{
    if (flags_remaining > 0)
        lseek(pmfd, flags_remaining, SEEK_CUR);
    Utils::readAll(pmfd, &area, sizeof(Area));
    next_pfd_offset = area.page_offset;
    current_addr = static_cast<char*>(area.addr);
    flag_i = 4096;
    if (area.skip) {
        flags_remaining = 0;
    } else {
        flags_remaining = area.size / 4096;
    }
}

Area& SaveState::getArea()
{
    return area;
}

char SaveState::getPageFlag(char* addr)
{
    /* If we already gathered the flag for this address, return it again */
    if (addr == (current_addr - 4096))
        return current_flag;

    while ((area.addr != nullptr) && (addr >= static_cast<char*>(area.endAddr))) {
        /* Skip areas until the one we are interested in */
        nextArea();
    }

    // debuglogstdio(LCF_CHECKPOINT, "Savestate addr query %p, current area %p and size %d, with current addr %p", addr, area.addr, area.size, current_addr);

    if (area.addr == nullptr)
        return Area::NONE;

    if (addr < static_cast<char*>(area.addr))
        return Area::NONE;

    if (area.skip)
        return Area::NONE;

    char flag;
    do {
        flag = nextFlag();
        if (flag == Area::FULL_PAGE) {
            next_pfd_offset += 4096;
        }
        current_addr += 4096;
    } while (current_addr <= addr);

    return flag;
}

/* Like getPageFlag(), but assumes you're going through the addresses
 * sequentially.  This means it can skip some checks and be a little faster. */
char SaveState::getNextPageFlag()
{
    char flag = nextFlag();
    if (flag == Area::FULL_PAGE) {
        next_pfd_offset += 4096;
    }
    current_addr += 4096;
    return flag;
}

void SaveState::finishLoad()
{
    if (queued_size > 0) {
        lseek(pfd, queued_offset, SEEK_SET);
        Utils::readAll(pfd, queued_addr, queued_size);
        queued_size = 0;
    }
}

void SaveState::queuePageLoad(char* addr)
{
    MYASSERT(addr + 4096 == current_addr);

    if (queued_size > 0) {
    	if ((next_pfd_offset - 4096) == queued_offset + queued_size &&
    	    addr == queued_addr + queued_size) {
            queued_size += 4096;
            return;
    	} else {
            finishLoad();
    	}
    }

    queued_offset = (next_pfd_offset - 4096);
    queued_addr = addr;
    queued_size = 4096;
}

}
