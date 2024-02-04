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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#ifndef LIBTAS_SAVESTATESAVING_H
#define LIBTAS_SAVESTATESAVING_H

#include "MemArea.h"
#include "../external/lz4.h"

namespace libtas {
    
struct StateHeader;

class SaveStateSaving
{
public:
    SaveStateSaving(int pagemapfd, int pagesfd, int selfpagemapfd);

    /* Import an area and fill some missing members */
    void processArea(Area area);
    
    Area getArea();

    /* Saving the page flag */
    void savePageFlag(char flag);
    
    /* Save the entire memory page and the associated page flag */
    int queuePageSave(char* addr);
    
    /* Finish processing a memory area */
    void finishSave();

private:
    /* Chunk of savestate pagemap values */
    char ss_pagemaps[4096];

    /* Current index in the savestate pagemap array */
    int ss_pagemap_i = 0;

    /* Compressed chunk */
    char compressed_page[LZ4_COMPRESSBOUND(4096)];
    
    LZ4_stream_t lz4s;

    int pmfd, pfd, spmfd;

    /* Address and size of the memory segment that is queued to be saved */
    char* queued_addr;
    int queued_size;

    Area area;
};
}

#endif
