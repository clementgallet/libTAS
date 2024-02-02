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

#include "SaveStateSaving.h"
// #include "StateHeader.h"

#include "Utils.h"
#include "logging.h"
#include "../external/lz4.h"
#include "global.h"
#include "GlobalState.h"

#include <fcntl.h>
#include <unistd.h>

namespace libtas {

SaveStateSaving::SaveStateSaving(int pagemapfd, int pagesfd, int selfpagemapfd)
{
    ss_pagemap_i = 0;

    pmfd = pagemapfd;
    pfd = pagesfd;
    spmfd = selfpagemapfd;

    LZ4_initStream(&lz4s, sizeof(lz4s));
}

void SaveStateSaving::processArea(Area a)
{
    area = a;
    
    /* Save the position of the first area page in the pages file */
    area.page_offset = lseek(pfd, 0, SEEK_CUR);
    MYASSERT(area.page_offset != -1)

    /* Write the area struct */
    area.skip = area.isSkipped();

    if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT)
        area.uncommitted = area.isUncommitted(spmfd);
    else
        area.uncommitted = false;
    
    Utils::writeAll(pmfd, &area, sizeof(area));
}

Area SaveStateSaving::getArea()
{
    return area;
}

void SaveStateSaving::savePageFlag(char flag)
{
    /* We write a chunk of savestate pagemaps if it is full */
    if (ss_pagemap_i >= 4096) {
        Utils::writeAll(pmfd, ss_pagemaps, 4096);
        ss_pagemap_i = 0;
    }

    ss_pagemaps[ss_pagemap_i++] = flag;
}

int SaveStateSaving::savePage(char* addr)
{
    int compressed_size = 0;
    if (Global::shared_config.savestate_settings & SharedConfig::SS_COMPRESSED) {
        compressed_size = LZ4_compress_fast_continue(&lz4s, addr, compressed_page, 4096, LZ4_COMPRESSBOUND(4096), 1);
    }
    if (compressed_size != 0) {
        savePageFlag(Area::COMPRESSED_PAGE);
        Utils::writeAll(pfd, &compressed_size, sizeof(int));
        Utils::writeAll(pfd, compressed_page, compressed_size);
    }
    else {
        compressed_size = 4096;
        savePageFlag(Area::FULL_PAGE);
        Utils::writeAll(pfd, static_cast<void*>(addr), 4096);
    }
    return compressed_size;
}

void SaveStateSaving::finishSave()
{
    /* Writing the last savestate pagemap chunk */
    Utils::writeAll(pmfd, ss_pagemaps, ss_pagemap_i);
}

}
