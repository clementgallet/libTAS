/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "SaveStateLoading.h"
#include "StateHeader.h"

#include "Utils.h"
#include "logging.h"
#include "../external/lz4.h"
#define XXH_INLINE_ALL
#define XXH_STATIC_LINKING_ONLY
#define XXH_NO_STDLIB
#define XXH_NO_STREAM
#include "../external/xxhash.h"

#include "global.h"
#include "GlobalState.h"

#include <fcntl.h>
#include <unistd.h>

/* Upper bound of possible page size values */
static const size_t MAX_PAGE_SIZE = 65536;

namespace libtas {

SaveStateLoading::SaveStateLoading(const char* pagemappath, const char* pagespath) : page_size(Utils::getPageSize())
{
    queued_size = 0;
    pmfd = -1;
    pfd = -1;

    if (pagemappath[0] == '\0') {
        return;
    }

    pmfd = open(pagemappath, O_RDONLY);
    MYASSERT(pmfd != -1)

    pfd = open(pagespath, O_RDONLY);
    MYASSERT(pfd != -1)

    memset(&lz4s, 0, sizeof(LZ4_streamDecode_t));
    restart();
}

SaveStateLoading::~SaveStateLoading()
{
    if (pmfd >= 0)
        close(pmfd);
    if (pfd >= 0)
        close(pfd);
}

bool SaveStateLoading::validateCompressedLength() const
{
    if (compressed_length <= 0 || compressed_length > LZ4_COMPRESSBOUND(page_size)) {
        LOG(LL_ERROR, LCF_CHECKPOINT, "Invalid compressed page length %d", compressed_length);
        return false;
    }
    return true;
}

void SaveStateLoading::readHeader(StateHeader* sh)
{
    lseek(pmfd, 0, SEEK_SET);
    Utils::readAll(pmfd, sh, sizeof(StateHeader));

    restart();
}

void SaveStateLoading::restart()
{
    /* Seek after the savestate header */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);
    flags_remaining = 0;

    /* Read the first area */
    nextArea();
}

char SaveStateLoading::nextFlag()
{
    if (flag_i == FLAGS_CHUNK) {
    	MYASSERT(flags_remaining > 0);

    	int size = (flags_remaining > FLAGS_CHUNK ? FLAGS_CHUNK : flags_remaining);

    	Utils::readAll(pmfd, flags, size);
    	flags_remaining -= size;

    	flag_i = 0;
    }

    current_flag = flags[flag_i++];
    return current_flag;
}

Area& SaveStateLoading::nextArea()
{
    if (flags_remaining > 0)
        lseek(pmfd, flags_remaining, SEEK_CUR);
    Utils::readAll(pmfd, &area, sizeof(area));
    next_pfd_offset = area.page_offset;
    current_addr = static_cast<char*>(area.addr);
    flag_i = FLAGS_CHUNK;
    if (area.skip || area.uncommitted) {
        flags_remaining = 0;
    } else {
        flags_remaining = (area.size + page_size - 1) / page_size;
    }
    LZ4_setStreamDecode(&lz4s, nullptr, 0);
    return area;
}

Area& SaveStateLoading::getArea()
{
    return area;
}

void SaveStateLoading::checkHash()
{
    if (Global::shared_config.logging_level < LL_DEBUG)
        return;
    
    // uint64_t hash = XXH3_64bits(area.addr, area.size);
    // 
    // if (hash != area.hash) {
    //     LOG(LL_WARN, LCF_CHECKPOINT, "     Area hash mismatch! (stored %llx, new %llx)", area.hash, hash);
    // }
}

char SaveStateLoading::getPageFlag(char* addr)
{
    /* If we already gathered the flag for this address, return it again */
    if (addr == (current_addr - Utils::getPageSize()))
        return current_flag;

    while ((area.addr != nullptr) && (addr >= static_cast<char*>(area.endAddr))) {
        /* Skip areas until the one we are interested in */
        nextArea();
    }

    // LOG(LL_DEBUG, LCF_CHECKPOINT, "Savestate addr query %p, current area %p and size %d, with current addr %p", addr, area.addr, area.size, current_addr);

    if (area.addr == nullptr)
        return Area::NONE;

    if (addr < static_cast<char*>(area.addr))
        return Area::NONE;

    if (area.skip || area.uncommitted)
        return Area::NONE;

    char flag;
    do {
        flag = nextFlag();
        if (flag == Area::FULL_PAGE) {
            next_pfd_offset += page_size;
        }
        else if (flag == Area::COMPRESSED_PAGE) {
            lseek(pfd, next_pfd_offset, SEEK_SET);
            Utils::readAll(pfd, &compressed_length, sizeof(int));
            if (!validateCompressedLength()) {
                current_flag = Area::NONE;
                return Area::NONE;
            }
            next_pfd_offset += sizeof(int) + compressed_length;
        }
        current_addr += page_size;
    } while (current_addr <= addr);

    return flag;
}

/* Like getPageFlag(), but assumes you're going through the addresses
 * sequentially.  This means it can skip some checks and be a little faster. */
char SaveStateLoading::getNextPageFlag()
{
    char flag = nextFlag();
    if (flag == Area::FULL_PAGE) {
        next_pfd_offset += page_size;
    }
    else if (flag == Area::COMPRESSED_PAGE) {
        lseek(pfd, next_pfd_offset, SEEK_SET);
        Utils::readAll(pfd, &compressed_length, sizeof(int));
        if (!validateCompressedLength()) {
            current_flag = Area::NONE;
            return Area::NONE;
        }
        next_pfd_offset += sizeof(int) + compressed_length;
    }
    current_addr += page_size;
    return flag;
}

void SaveStateLoading::finishLoad()
{
    if (queued_size > 0) {
        lseek(pfd, queued_offset, SEEK_SET);
        Utils::readAll(pfd, queued_addr, queued_size);
        queued_size = 0;
    }
}

void SaveStateLoading::queuePageLoad(char* addr)
{
    // MYASSERT(addr + page_size == current_addr);

    if (current_flag == Area::FULL_PAGE) {
        if (queued_size > 0) {
        	if ((next_pfd_offset - page_size) == queued_offset + queued_size &&
        	    addr == queued_addr + queued_size) {
                queued_size += page_size;
                return;
        	} else {
                lseek(pfd, queued_offset, SEEK_SET);
                Utils::readAll(pfd, queued_addr, queued_size);
        	}
        }
        queued_offset = (next_pfd_offset - page_size);
        queued_addr = addr;
        queued_size = page_size;
    }
    else if (current_flag == Area::COMPRESSED_PAGE) {
        char compressed[LZ4_COMPRESSBOUND(MAX_PAGE_SIZE)];
        if (!validateCompressedLength()) {
            memset(addr, 0, page_size);
            return;
        }
        Utils::readAll(pfd, compressed, compressed_length);
        
        if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
            /* For incremental savestates, block compression is independant */
            int ret = LZ4_decompress_safe(compressed, addr, compressed_length, page_size);
            if (ret != page_size) {
                LOG(LL_ERROR, LCF_CHECKPOINT, "LZ4_decompress_safe failed with return code %d", ret);
                memset(addr, 0, page_size);
            }
        }
        else {
            int ret = LZ4_decompress_safe_continue(&lz4s, compressed, addr, compressed_length, page_size);
            if (ret != page_size) {
                LOG(LL_ERROR, LCF_CHECKPOINT, "LZ4_decompress_safe_continue failed with return code %d", ret);
                memset(addr, 0, page_size);
            }
        }
    }
}

bool SaveStateLoading::debugIsMatchingPage(char* addr)
{
    char current_page[MAX_PAGE_SIZE];
    
    if (current_flag == Area::FULL_PAGE) {
        lseek(pfd, next_pfd_offset - page_size, SEEK_SET);
        Utils::readAll(pfd, current_page, page_size);
    }
    else if (current_flag == Area::COMPRESSED_PAGE) {
        char compressed[LZ4_COMPRESSBOUND(MAX_PAGE_SIZE)];
        if (!validateCompressedLength()) {
            memset(current_page, 0, page_size);
            return false;
        }
        Utils::readAll(pfd, compressed, compressed_length);
        int ret = LZ4_decompress_safe(compressed, current_page, compressed_length, page_size);
        if (ret != page_size) {
            LOG(LL_ERROR, LCF_CHECKPOINT, "LZ4_decompress_safe failed with return code %d", ret);
            memset(current_page, 0, page_size);
            return false;
        }
    }
    
    return 0 == memcmp(addr, current_page, page_size);
}

}
