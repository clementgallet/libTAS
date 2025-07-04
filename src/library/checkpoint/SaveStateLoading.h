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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#ifndef LIBTAS_SAVESTATELOADING_H
#define LIBTAS_SAVESTATELOADING_H

#include "MemArea.h"
#include "../external/lz4.h"

namespace libtas {
    
struct StateHeader;

class SaveStateLoading
{
    public:
        SaveStateLoading(const char* pagemappath, const char* pagespath);
        ~SaveStateLoading();

    // Also resets back to first area
    void readHeader(StateHeader* sh);

    Area& getArea();
    Area& nextArea();

    void checkHash();
    
    // Reset back to first area
    void restart();

    char getPageFlag(char* addr);
    char getNextPageFlag();
    void queuePageLoad(char* addr);
    void finishLoad();

    bool debugIsMatchingPage(char* addr);

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

    int compressed_length;
    char* queued_addr;
    off_t queued_offset;
    int queued_size;
    LZ4_streamDecode_t lz4s;
};
}

#endif
