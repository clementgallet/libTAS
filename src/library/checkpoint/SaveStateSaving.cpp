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

#include "SaveStateSaving.h"
#include "ReservedMemory.h"

#include "Utils.h"
#include "logging.h"
#include "../external/lz4.h"
#include "fileio/SaveFile.h"
#include "fileio/SaveFileList.h"

#define XXH_INLINE_ALL
#define XXH_STATIC_LINKING_ONLY
#define XXH_NO_STDLIB
#define XXH_NO_STREAM
#include "../external/xxhash.h"
#include "global.h"
#include "GlobalState.h"

#include <fcntl.h>
#include <unistd.h>

namespace libtas {

SaveStateSaving::SaveStateSaving(int pagemapfd, int pagesfd, int selfpagemapfd)
{
    ss_pagemap_i = 0;
    queued_size = 0;

    queued_compressed_base_addr = static_cast<char*>(ReservedMemory::getAddr(ReservedMemory::COMPRESSED_ADDR));
    queued_compressed_max_size = ReservedMemory::COMPRESSED_SIZE;
    queued_compressed_size = 0;
    queued_target_addr = nullptr;

    pmfd = pagemapfd;
    pfd = pagesfd;
    spmfd = selfpagemapfd;

    LZ4_initStream(&lz4s, sizeof(lz4s));
}

void SaveStateSaving::processArea(Area* area)
{
    /* Save the position of the first area page in the pages file */
    area->page_offset = lseek(pfd, 0, SEEK_CUR);
    MYASSERT(area->page_offset != -1)

    /* Write the area struct */
    if (!area->skip)
        area->skip = area->isSkipped();

    if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT)
        area->uncommitted = area->isUncommitted(spmfd);
    else
        area->uncommitted = false;

    if (Global::shared_config.logging_level >= LL_DEBUG) {
        if (area->skip || area->uncommitted)
            area->hash = 0;
        else
            area->hash = XXH3_64bits(area->addr, area->size);
    }

    /* Little hack, set addr to NULL for memfd, so that state loading won't 
     * process this area, without extra code. We needed a valid address until now
     * for computing the hash. */
    void* old_addr = area->addr;
    if (area->flags & Area::AREA_MEMFD)
        area->addr = nullptr;

    Utils::writeAll(pmfd, area, sizeof(*area));

    area->addr = old_addr;
    
    LZ4_resetStream_fast(&lz4s);
}

void SaveStateSaving::savePageFlag(char flag)
{
    /* We write a chunk of savestate pagemaps if it is full */
    if (ss_pagemap_i >= PAGEMAP_CHUNK) {
        Utils::writeAll(pmfd, ss_pagemaps, PAGEMAP_CHUNK);
        ss_pagemap_i = 0;
    }

    ss_pagemaps[ss_pagemap_i++] = flag;
}

size_t SaveStateSaving::queuePageSave(char* addr)
{
    size_t returned_size = 0;
    
    if (Global::shared_config.savestate_settings & SharedConfig::SS_COMPRESSED) {
        /* Try to compress the memory page */
        if ((queued_compressed_size > 0) && (addr != queued_target_addr)) {
            /* Flush current buffer */
            returned_size = flushCompressedSave();
        }
            
        /* Append the compressed data to the current stream */
        int compressed_size;
        if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
            /* For incremental savestates, not all blocks may be decompressed, so
             * we must compress each block independantly */
            compressed_size = LZ4_compress_fast(addr, queued_compressed_base_addr + queued_compressed_size + sizeof(int), 4096, queued_compressed_max_size - (queued_compressed_size + sizeof(int)), 1);
        }
        else {
            compressed_size = LZ4_compress_fast_continue(&lz4s, addr, queued_compressed_base_addr + queued_compressed_size + sizeof(int), 4096, queued_compressed_max_size - (queued_compressed_size + sizeof(int)), 1);
        }
        if (compressed_size) {
            /* Flush the uncompressed buffer if any */
            returned_size = flushSave();

            savePageFlag(Area::COMPRESSED_PAGE);
            memcpy(queued_compressed_base_addr + queued_compressed_size, &compressed_size, sizeof(int));
            queued_compressed_size += compressed_size + sizeof(int);
            queued_target_addr = addr + 4096;

            /* Check for remaining size */
            if ((queued_compressed_max_size - queued_compressed_size) < LZ4_COMPRESSBOUND(4096)) {
                returned_size += flushCompressedSave();
            }
            return returned_size;
        }
        else {
            /* Could not compress the memory page. Write the whole buffer and 
             * fallback to the second part of the code to write the regular page */
            returned_size += flushCompressedSave();
        }
    }

    /* Save regular memory page */
    savePageFlag(Area::FULL_PAGE);
    
    /* Try to queue the page save, to reduce the number of calls */
    if (queued_size > 0) {
        if (addr == (queued_addr + queued_size)) {
            queued_size += 4096;
            return 0;
        } else {
            returned_size += flushSave();
        }
    }
    queued_addr = addr;
    queued_size = 4096;

    return returned_size;
}

size_t SaveStateSaving::flushSave()
{
    if (queued_size > 0) {
        Utils::writeAll(pfd, queued_addr, queued_size);
        int returned_size = queued_size;
        queued_size = 0;
        return returned_size;
    }
    return 0;
}

size_t SaveStateSaving::flushCompressedSave()
{
    if (queued_compressed_size > 0) {
        Utils::writeAll(pfd, queued_compressed_base_addr, queued_compressed_size);
        int returned_size = queued_compressed_size;
        queued_compressed_size = 0;
        return returned_size;        
    }
    return 0;
}

size_t SaveStateSaving::finishSave()
{
    size_t returned_size = 0;
    
    /* We don't care about the following order of the saves, because code
     * guarantees that at most one of those has non-zero queue size. */
    returned_size += flushSave();
    returned_size += flushCompressedSave();
    
    /* Writing the last savestate pagemap chunk */
    Utils::writeAll(pmfd, ss_pagemaps, ss_pagemap_i);
    ss_pagemap_i = 0;
    
    return returned_size;
}

}
