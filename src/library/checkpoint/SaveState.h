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

#ifndef LIBTAS_SAVESTATE_H
#define LIBTAS_SAVESTATE_H

#include "ProcMapsArea.h"
#include "StateHeader.h"

namespace libtas {
class SaveState
{
    public:
        SaveState(const char* pagemappath, const char* pagespath, int pagemapfd, int pagesfd);
        ~SaveState();

	// Also resets back to first area
	void readHeader(StateHeader& sh);

	Area& getArea();
	void nextArea();

	// Reset back to first area
	void restart();

    char getPageFlag(char* addr);
	char getNextPageFlag();
	void queuePageLoad(char* addr);
	void finishLoad();

    explicit operator bool() const {
        return (pmfd != -1);
    }

    private:
	char nextFlag();

	char flags[4096];
    char current_flag;
	int flag_i;
	int flags_remaining;

    int pmfd, pfd;

    Area area;
    char* current_addr;
	off_t next_pfd_offset;

	char* queued_addr;
	off_t queued_offset;
	int queued_size;
};
}

#endif
