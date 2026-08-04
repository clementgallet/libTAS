/*
    Copyright 2015-2026 Clement Gallet <clement.gallet@ens-lyon.org>

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
*/

#include "CheckpointSavefiles.h"

#include "Utils.h"
#include "SaveStateLoading.h"
#include "SaveStateSaving.h"
#include "MemArea.h"
#include "logging.h"
#include "fileio/SaveFile.h"
#include "fileio/SaveFileList.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <memory>
#include <cstring>

namespace libtas {

void readASavefile(SaveStateLoading& saved_state)
{
    const Area& saved_area = saved_state.getArea();

    if (!(saved_area.flags & Area::AREA_SAVEFILE))
        return;

    int ret = ftruncate(saved_area.fd, saved_area.size);

    if (ret < 0) {
        LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not truncate the savefile %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);
        return;
    }

    void* saved_area_addr = mmap(nullptr, saved_area.size, PROT_WRITE, MAP_SHARED, saved_area.fd, 0);

    if (saved_area_addr == MAP_FAILED) {
        LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not map file %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);
        return;
    }

    LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "Restore savefile %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);

    /* Map the original file */
    int orig_fd = -1;
    void* orig_file_mapped_addr = MAP_FAILED;

    struct stat filestat;
    int rv = stat(saved_area.name, &filestat);

    if (rv < 0)
        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     No original savefile to use");
    else if (! S_ISREG(filestat.st_mode) || (filestat.st_size == 0))
        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "    Original file is missing or not regular, skip it");
    else {
        orig_fd = open(saved_area.name, O_RDONLY);

        if (orig_fd == -1)
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not locate original savefile");
        else {
            orig_file_mapped_addr = mmap(nullptr, filestat.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_FILE, orig_fd, 0);
            close(orig_fd);
        }
    }

    /* Go through both original and current mapped files, and only store
     * pages that are different */
    char* mapped_addr_begin = static_cast<char*>(saved_area_addr);
    char* mapped_addr_end = mapped_addr_begin + saved_area.size;
    bool has_orig_file_mapping = (orig_file_mapped_addr != MAP_FAILED);
    char* orig_file_mapped_begin = has_orig_file_mapping ? static_cast<char*>(orig_file_mapped_addr) : nullptr;
    char* orig_file_mapped_end = has_orig_file_mapping ? (orig_file_mapped_begin + filestat.st_size) : nullptr;
    off_t page_size = Utils::getPageSize();

    for (;mapped_addr_begin < mapped_addr_end; mapped_addr_begin += page_size) {
        size_t page_len = (mapped_addr_end - mapped_addr_begin) > page_size ? page_size : (mapped_addr_end - mapped_addr_begin);

        char flag = saved_state.getNextPageFlag();

        if (flag == Area::FILE_PAGE) {
            /* Copy the original file into this file */
            if (has_orig_file_mapping && orig_file_mapped_begin < orig_file_mapped_end) {
                size_t orig_page_len = orig_file_mapped_end - orig_file_mapped_begin;
                if (orig_page_len > page_len)
                    orig_page_len = page_len;
                memcpy(mapped_addr_begin, orig_file_mapped_begin, orig_page_len);
                if (orig_page_len < page_len)
                    memset(mapped_addr_begin + orig_page_len, 0, page_len - orig_page_len);
            }
            else {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "     Page %p is not stored but original file is missing!", mapped_addr_begin);
            }
        }
        else {
            saved_state.queuePageLoad(mapped_addr_begin);
        }

        if (has_orig_file_mapping)
            orig_file_mapped_begin += page_size;
    }

    saved_state.finishLoad();

    saved_state.getArea().addr = saved_area_addr;
    saved_state.checkHash();

    /* Unmap files */
    if (saved_area_addr != MAP_FAILED)
        munmap(saved_area_addr, saved_area.size);

    if (orig_file_mapped_addr != MAP_FAILED)
        munmap(orig_file_mapped_addr, filestat.st_size);
}

/* Write savefiles into the savestate. Returns the total size in bytes */
size_t writeSaveFiles(SaveStateSaving& state)
{
    size_t total_size = 0;

    for (auto it = SaveFileList::begin(); it != SaveFileList::end(); it++) {
        const std::unique_ptr<SaveFile>& savefile = *it;
        /* Don't map if file was closed and removed (TODO: handle this!) */
        if (savefile->fd == 0)
            continue;

        /* Also, skip if we are handling the savefile using our custom
         * stream (SaveFileStream), which maps its content in memory. So, it
         * is already saved through the memory dumping process. */
        if (savefile->fd >= FIRST_SAVEFILE_STREAM_FD)
            continue;

        Area area;
        area.flags = Area::AREA_SAVEFILE;
        area.fd = savefile->fd;
        strncpy(area.name, savefile->filename.c_str(), Area::FILENAMESIZE-1);

        /* Get current file length and map at least this amount of bytes */
        struct stat filestat;
        int rv = fstat(area.fd, &filestat);

        if (rv < 0)
            continue;

        area.size = filestat.st_size;

        if (area.size == 0) {
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Don't map empty/missing file %s with fd %d", savefile->filename.c_str(), savefile->fd);
            continue;
        }

        area.addr = mmap(nullptr, area.size, PROT_READ, MAP_PRIVATE | MAP_FILE, savefile->fd, 0);

        if (area.addr == MAP_FAILED) {
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not map file %s with fd %d and size %zu", savefile->filename.c_str(), savefile->fd, area.size);
            continue;
        }

        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "Save savefile %s with fd %d and size %zu", savefile->filename.c_str(), savefile->fd, area.size);

        /* Map the original file */
        int orig_fd = -1;
        size_t orig_file_mapped_size = 0;
        void* orig_file_mapped_addr = MAP_FAILED;

        rv = stat(savefile->filename.c_str(), &filestat);

        if (rv < 0)
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     No original savefile to use");
        else if (! S_ISREG(filestat.st_mode) || (filestat.st_size == 0))
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "    Original file is missing or not regular, skip it");
        else {
            orig_fd = open(savefile->filename.c_str(), O_RDONLY);

            if (orig_fd == -1)
                LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not locate original savefile");
            else {
                orig_file_mapped_size = filestat.st_size;
                orig_file_mapped_addr = mmap(nullptr, filestat.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_FILE, orig_fd, 0);
                if (orig_file_mapped_addr == MAP_FAILED)
                    orig_file_mapped_size = 0;
                close(orig_fd);
            }
        }

        /* Go through both original and current mapped files, and only store
         * pages that are different */
        char* mapped_addr_begin = static_cast<char*>(area.addr);
        char* mapped_addr_end = mapped_addr_begin + area.size;
        bool has_orig_file_mapping = (orig_file_mapped_addr != MAP_FAILED);
        char* orig_file_mapped_begin = has_orig_file_mapping ? static_cast<char*>(orig_file_mapped_addr) : nullptr;
        char* orig_file_mapped_end = has_orig_file_mapping ? (orig_file_mapped_begin + orig_file_mapped_size) : nullptr;
        off_t page_size = Utils::getPageSize();

        /* Stats to print */
        int pagecount_full = 0;
        int pagecount_file = 0;

        state.processArea(&area);
        size_t area_size = sizeof(area);

        for (;mapped_addr_begin < mapped_addr_end; mapped_addr_begin += page_size) {
            size_t page_len = (mapped_addr_end - mapped_addr_begin) > page_size ? page_size : (mapped_addr_end - mapped_addr_begin);

            /* Check difference with original file */
            if (has_orig_file_mapping) {

                /* If we reached the end of the original file, save all remaining pages */
                if (orig_file_mapped_begin >= orig_file_mapped_end) {
                    area_size += state.queuePageSave(mapped_addr_begin);
                    pagecount_full++;
                }
                else {
                    size_t orig_page_len = orig_file_mapped_end - orig_file_mapped_begin;
                    if (orig_page_len > page_len)
                        orig_page_len = page_len;

                    int diff = memcmp(mapped_addr_begin, orig_file_mapped_begin, orig_page_len);
                    if ((diff == 0) && (orig_page_len < page_len)) {
                        for (size_t i = orig_page_len; i < page_len; i++) {
                            if (mapped_addr_begin[i] != 0) {
                                diff = 1;
                                break;
                            }
                        }
                    }
                    if (diff == 0) {
                        state.savePageFlag(Area::FILE_PAGE);
                        pagecount_file++;
                    }
                    else {
                        area_size += state.queuePageSave(mapped_addr_begin);
                        pagecount_full++;
                    }
                }
            }
            else {
                area_size += state.queuePageSave(mapped_addr_begin);
                pagecount_full++;
            }

            if (has_orig_file_mapping)
                orig_file_mapped_begin += page_size;
        }

        area_size += state.finishSave();

        /* Add the number of page flags to the total size */
        area_size += (area.size + page_size - 1) / page_size;

        LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, file: %d. Size %zu", pagecount_full, pagecount_file, area_size);

        total_size += area_size;

        /* Unmap files */
        if (area.addr != MAP_FAILED)
            munmap(area.addr, area.size);

        if (orig_file_mapped_addr != MAP_FAILED)
            munmap(orig_file_mapped_addr, filestat.st_size);

    }

    return total_size;
}

}
