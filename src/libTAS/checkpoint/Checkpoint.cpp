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

#include "Checkpoint.h"
#include "ThreadManager.h"
#include "../logging.h"
#include "ProcMapsArea.h"
#include "ProcSelfMaps.h"
#include "StateHeader.h"
#include "Utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <X11/Xlibint.h>
#include <X11/Xlib-xcb.h>
#include <sys/statvfs.h>
#include "errno.h"
#include "../../external/xcbint.h"
#include "../renderhud/RenderHUD.h"
#include "ReservedMemory.h"
#include "SaveState.h"

#define ONE_MB 1024 * 1024

namespace libtas {

static const char* pagemappath;
static const char* pagespath;

static char parentpagemappath[1024];
static char parentpagespath[1024];

static char basepagemappath[1024];
static char basepagespath[1024];

static bool skipArea(const Area *area);

static void readAllAreas();
static int reallocateArea(Area *saved_area, Area *current_area);
static void readAnArea(const Area &saved_area, int pmfd, int pfd, SaveState &parent_state, SaveState &base_state);

static void writeAllAreas(bool base);
static void writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state, SaveState &base_state);

void Checkpoint::setSavestatePath(const char* pmpath, const char* pspath)
{
    /* We want to avoid any allocated memory here, so using char array */
    pagemappath = pmpath;
    pagespath = pspath;
}

void Checkpoint::setBaseSavestatePath(std::string path)
{
    std::string pmpath = path + ".pm";
    strncpy(basepagemappath, pmpath.c_str(), 1023);

    std::string ppath = path + ".p";
    strncpy(basepagespath, ppath.c_str(), 1023);
}

void Checkpoint::setParentSavestatePath(std::string path)
{
    std::string pmpath = path + ".pm";
    strncpy(parentpagemappath, pmpath.c_str(), 1023);

    std::string ppath = path + ".p";
    strncpy(parentpagespath, ppath.c_str(), 1023);
}

bool Checkpoint::checkCheckpoint()
{
    /* Get an estimation of the savestate space */
    size_t savestate_size = 0;

    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(0), ONE_MB);

    /* Read the first current area */
    Area area;
    bool not_eof = procSelfMaps.getNextArea(&area);

    while (not_eof) {
        if (!skipArea(&area)) {
            savestate_size += area.size;
        }
        not_eof = procSelfMaps.getNextArea(&area);
    }

    /* Get the savestate directory */
    std::string savestate_str = pagemappath;
    size_t sep = savestate_str.find_last_of("/");
    if (sep != std::string::npos)
        savestate_str.resize(sep);

    struct statvfs devData;
    int ret;
    if ((ret = statvfs(savestate_str.c_str(), &devData)) >= 0) {
        unsigned long available_size = devData.f_bavail * devData.f_bsize;
        if (savestate_size > available_size) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Not enough available space to store the savestate");
            RenderHUD::insertMessage("Not enough available space to store the savestate");
            return false;
        }
    }
    // else {
    //     debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "statvfs errno gives %d", errno);
    // }

    return true;
}

bool Checkpoint::checkRestore()
{
    /* Check that the savestate files exist */
    struct stat sb;
    if (stat(pagemappath, &sb) == -1)
        return false;
    if (stat(pagespath, &sb) == -1)
        return false;

    int pmfd;
    NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
    if (pmfd == -1)
        return false;

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));
    NATIVECALL(close(pmfd));

    /* Check that the thread list is identical */
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
        if ((thread->state != ThreadInfo::ST_RUNNING) &&
            (thread->state != ThreadInfo::ST_ZOMBIE) &&
            (thread->state != ThreadInfo::ST_FREE))
            continue;

        int t;
        for (t=0; t<sh.thread_count; t++) {
            if (sh.pthread_ids[t] == thread->pthread_id && (sh.tids[t] == thread->tid)) {
                n++;
                break;
            }
        }

        if (t == sh.thread_count) {
            /* We didn't find a match */
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
            RenderHUD::insertMessage("Loading the savestate not allowed because new threads where created");
            return false;
        }
    }

    if (n != sh.thread_count) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
        RenderHUD::insertMessage("Loading the savestate not allowed because new threads where created");
        return false;
    }

    return true;
}

void Checkpoint::handler(int signum)
{
    /* Access the X Window identifier from the SDL_Window struct */
    Display *display = gameDisplay;
    if (!display) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Could not access connection to X11");
        return;
    }

    /* Check that we are using our alternate stack by looking at the address
     * of this local variable.
     */
    if ((&display < ReservedMemory::getAddr(0)) ||
        (&display >= ReservedMemory::getAddr(ReservedMemory::getSize()))) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Checkpoint code is not running on alternate stack");
        return;
    }

    XSync(display, false);

    if (ThreadManager::restoreInProgress) {
        /* Before reading from the savestate, we must keep some values from
         * the connection to the X server, because they are used for checking
         * consistency of requests. We can store them in this (alternate) stack
         * which will be preserved.
         */

#ifdef X_DPY_GET_LAST_REQUEST_READ
        uint64_t last_request_read = X_DPY_GET_LAST_REQUEST_READ(display);
        uint64_t request = X_DPY_GET_REQUEST(display);
#else
        uint64_t last_request_read = static_cast<uint64_t>(display->last_request_read);
        uint64_t request = static_cast<uint64_t>(display->request);
#endif

        /* Copy the entire xcb connection struct */
        xcb_connection_t xcb_conn;
        xcb_connection_t *cur_xcb_conn = XGetXCBConnection(display);
        memcpy(&xcb_conn, cur_xcb_conn, sizeof(xcb_connection_t));

        readAllAreas();
        /* restoreInProgress was overwritten, putting the right value again */
        ThreadManager::restoreInProgress = true;

        /* Restoring the display values */
#ifdef X_DPY_SET_LAST_REQUEST_READ
        X_DPY_SET_LAST_REQUEST_READ(display, last_request_read);
        X_DPY_SET_REQUEST(display, request);
#else
        display->last_request_read = static_cast<unsigned long>(last_request_read);
        display->request = static_cast<unsigned long>(request);
#endif

        /* Restore the entire xcb connection struct */
        memcpy(cur_xcb_conn, &xcb_conn, sizeof(xcb_connection_t));
    }
    else {
        /* Check that base savestate exists, otherwise save it */
        struct stat sb;
        if (stat(basepagemappath, &sb) == -1) {
            writeAllAreas(true);
        }

        writeAllAreas(false);
    }
    debuglogstdio(LCF_CHECKPOINT, "End restore.");
}

static bool skipArea(const Area *area)
{
    /* If it's readable, but it's VDSO, it will be dangerous to restore it.
    * In 32-bit mode later Red Hat RHEL Linux 2.6.9 releases use 0xffffe000,
    * the last page of virtual memory.  Note 0xffffe000 >= HIGHEST_VA
    * implies we're in 32-bit mode.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (void*)0xffffe000) {
        return true;
    }
    #ifdef __x86_64__

    /* And in 64-bit mode later Red Hat RHEL Linux 2.6.9 releases
    * use 0xffffffffff600000 for VDSO.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (void*)0xffffffffff600000) {
        return true;
    }
    #endif // ifdef __x86_64__

    if (strstr(area->name, "(deleted)")) {
        return true;
    }

    if (area->size == 0) {
        /* Kernel won't let us munmap this.  But we don't need to restore it. */
        return true;
    }

    if (0 == strcmp(area->name, "[vsyscall]") ||
    0 == strcmp(area->name, "[vectors]") ||
    0 == strcmp(area->name, "[vvar]") ||
    0 == strcmp(area->name, "[vdso]")) {
        return true;
    }

    if ((area->addr == ReservedMemory::getAddr(0)) && (area->size == ReservedMemory::getSize())) {
        return true;
    }

    /* Start of user-configurable skips */

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_NON_WRITEABLE) &&
        !(area->prot & PROT_WRITE)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_NON_ANONYMOUS_NON_WRITEABLE) &&
        !(area->prot & PROT_WRITE) && !(area->flags & MAP_ANONYMOUS)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_EXEC) &&
        (area->prot & PROT_EXEC)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_SHARED) &&
        (area->flags & MAP_SHARED)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_LARGE) &&
        (area->flags & MAP_ANONYMOUS) &&
        (area->size > 64*ONE_MB)) {
        return true;
    }

    return false;
}

static void readAllAreas()
{
    int pmfd;
    NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
    MYASSERT(pmfd != -1)

    int pfd;
    NATIVECALL(pfd = open(pagespath, O_RDONLY));
    MYASSERT(pfd != -1)

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));

    Area current_area;
    Area saved_area;

    debuglogstdio(LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(0), ONE_MB);

    /* Read the first saved area */
    Utils::readAll(pmfd, &saved_area, sizeof(Area));
    if (!(saved_area.properties & Area::SKIP))
        lseek(pmfd, saved_area.size / 4096, SEEK_CUR);

    /* Read the first current area */
    bool not_eof = procSelfMaps.getNextArea(&current_area);

    /* Reallocate areas to match the savestate areas. Nothing is written yet */
    while ((saved_area.addr != nullptr) || not_eof) {

        /* Check for matching areas */
        int cmp = reallocateArea(&saved_area, &current_area);
        if (cmp == 0) {
            /* Areas matched, we advance both areas */
            Utils::readAll(pmfd, &saved_area, sizeof(Area));
            if (!(saved_area.properties & Area::SKIP))
                lseek(pmfd, saved_area.size / 4096, SEEK_CUR);
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp > 0) {
            /* Saved area is greater, advance current area */
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp < 0) {
            /* Saved area is smaller, advance saved area */
            Utils::readAll(pmfd, &saved_area, sizeof(Area));
            if (!(saved_area.properties & Area::SKIP))
                lseek(pmfd, saved_area.size / 4096, SEEK_CUR);
        }
    }

    /* Now that the memory layout matches the savestate, we load savestate into memory */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);

    /* Load base and parent savestates */
    SaveState parent_state(parentpagemappath, parentpagespath);
    SaveState base_state(basepagemappath, basepagespath);

    /* Read the first saved area */
    Utils::readAll(pmfd, &saved_area, sizeof saved_area);

    while (saved_area.addr != nullptr) {
        readAnArea(saved_area, pmfd, pfd, parent_state, base_state);
        Utils::readAll(pmfd, &saved_area, sizeof(saved_area));
    }

    /* Clear soft-dirty bits */
    int fd;
    NATIVECALL(fd = open("/proc/self/clear_refs", O_WRONLY));
    MYASSERT(fd != -1);
    MYASSERT(write(fd, "4\n", 2) == 2);
    NATIVECALL(close(fd));

    /* That's all folks */
    NATIVECALL(close(pmfd));
    NATIVECALL(close(pfd));
}

static int reallocateArea(Area *saved_area, Area *current_area)
{
    /* Do Areas start on the same address? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
        (saved_area->addr == current_area->addr)) {

        /* Check if it is a skipped area */
        if (saved_area->properties & Area::SKIP) {
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Current section is not skipped anymore !?");
                saved_area->print("Saved");
                current_area->print("Current");
            }
            return 0;
        }

        saved_area->print("Restore");

        size_t copy_size = (saved_area->size<current_area->size)?saved_area->size:current_area->size;

        if (saved_area->size != current_area->size) {

            /* Special case for stacks, always try to resize the Area */
            if (strstr(saved_area->name, "[stack")) {
                debuglogstdio(LCF_CHECKPOINT, "Changing stack size from %d to %d", current_area->size, saved_area->size);
                void *newAddr = mremap(current_area->addr, current_area->size, saved_area->size, 0);

                if (newAddr == MAP_FAILED) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Resizing failed");
                    return 0;
                }

                if (newAddr != saved_area->addr) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "mremap relocated the area");
                    return 0;
                }

                copy_size = saved_area->size;
            }

            /* Special case for heap, use brk instead */
            if (strcmp(saved_area->name, "[heap]") == 0) {
                debuglogstdio(LCF_CHECKPOINT, "Changing heap size from %d to %d", current_area->size, saved_area->size);

                int ret = brk(saved_area->endAddr);

                if (ret < 0) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "brk failed");
                    return 0;
                }

                copy_size = saved_area->size;
            }
        }

        if ((saved_area->endAddr == current_area->endAddr) || (saved_area->name[0] == '[')) {
            /* If the Areas have the same size, or it was a special section,
             * we have nothing to do.
             */
            return 0;
        }

        if (saved_area->endAddr > current_area->endAddr) {
            /* If there is still some memory in the savestate to write,
             * we update the area
             */
            saved_area->addr = current_area->endAddr;
            saved_area->size -= copy_size;
            return 1;
        }

        if (saved_area->endAddr < current_area->endAddr) {
            /* There is extra memory that we have to deal with */
            current_area->addr = saved_area->endAddr;
            current_area->size -= copy_size;
            return -1;
        }
    }

    if ((saved_area->addr == nullptr) || (saved_area->addr > current_area->addr)) {
        /* Our current area starts before the saved area */

        if ((saved_area->addr != nullptr) && (current_area->endAddr > saved_area->addr)) {
            /* Areas are overlapping, we only unmap the non-shared region */
            ptrdiff_t unmap_size = reinterpret_cast<ptrdiff_t>(saved_area->addr) - reinterpret_cast<ptrdiff_t>(current_area->addr);

            /* We only deallocate this section if we are interested in it */
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, unmap_size);
                MYASSERT(munmap(current_area->addr, unmap_size) == 0)
            }

            /* We call this function again with the rest of the area */
            current_area->addr = saved_area->addr;
            current_area->size -= unmap_size;
            return reallocateArea(saved_area, current_area);
        }
        else {
            /* Areas are not overlapping, we unmap the whole area */
            /* We only deallocate this section if we are interested in it */
            if (!skipArea(current_area)) {
                current_area->print("Deallocating");
                MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            }
            return 1;
        }
    }

    if ((current_area->addr == nullptr) || (saved_area->addr < current_area->addr)) {

        if (saved_area->properties & Area::SKIP) {
            // debuglogstdio(LCF_CHECKPOINT, "Section is skipped");
            return -1;
        }

        size_t map_size = saved_area->size;
        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* Areas are overlapping, we only map the non-shared region */
            map_size = reinterpret_cast<ptrdiff_t>(current_area->addr) - reinterpret_cast<ptrdiff_t>(saved_area->addr);
        }

        /* This saved area must be allocated */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be allocated", saved_area->addr, saved_area->name, map_size);

        int imagefd = -1;
        if (!(saved_area->flags & MAP_ANONYMOUS)) {
            /* We shouldn't be creating any special section such as [heap] or [stack] */
            MYASSERT(saved_area->name[0] == '/')

            NATIVECALL(imagefd = open(saved_area->name, O_RDONLY, 0));
            if (imagefd >= 0) {
                /* If the current file size is smaller than the original, we map the region
                * as private anonymous. Note that with this we lose the name of the region
                * but most applications may not care.
                */
                off_t curr_size = lseek(imagefd, 0, SEEK_END);
                MYASSERT(curr_size != -1);
                if (static_cast<size_t>(curr_size) < saved_area->offset + map_size) {
                    NATIVECALL(close(imagefd));
                    imagefd = -1;
                    saved_area->offset = 0;
                    saved_area->flags |= MAP_ANONYMOUS;
                }
            }
            else {
                /* We could not open the file, map the section as anonymous */
                saved_area->flags |= MAP_ANONYMOUS;
            }
        }

        if (saved_area->flags & MAP_ANONYMOUS) {
            debuglogstdio(LCF_CHECKPOINT, "Restoring anonymous area, %d bytes at %p", map_size, saved_area->addr);
        } else {
            debuglogstdio(LCF_CHECKPOINT, "Restoring non-anonymous area, %d bytes at %p from %s + %d", map_size, saved_area->addr, saved_area->name, saved_area->offset);
        }

        /* Create the memory area */

        void *mmappedat = mmap(saved_area->addr, map_size, saved_area->prot,
            saved_area->flags, imagefd, saved_area->offset);

        if (mmappedat == MAP_FAILED) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Mapping %d bytes at %p failed", saved_area->size, saved_area->addr);
            return -1;
        }
        if (mmappedat != saved_area->addr) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
            return -1;
        }

        /* Close image file (fd only gets in the way) */
        if (imagefd >= 0) {
            NATIVECALL(close(imagefd));
        }

        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* If areas were overlapping, we must deal with the rest of the area */
            saved_area->addr = current_area->addr;
            saved_area->size -= map_size;
            return reallocateArea(saved_area, current_area);
        }

        return -1;
    }

    MYASSERT(false)
    return 0;
}

static void readAnArea(const Area &saved_area, int pmfd, int pfd, SaveState &parent_state, SaveState &base_state)
{
    if (saved_area.properties & Area::SKIP)
        return;

    MYASSERT(saved_area.page_offset == lseek(pfd, 0, SEEK_CUR))

    /* Add write permission to the area */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot | PROT_WRITE) == 0)
    }

    int fd;
    NATIVECALL(fd = open("/proc/self/pagemap", O_RDONLY));
    MYASSERT(fd != -1);

    char* endAddr = static_cast<char*>(saved_area.endAddr);
    for (char* curAddr = static_cast<char*>(saved_area.addr);
    curAddr < endAddr;
    curAddr += 4096) {

        // debuglogstdio(LCF_CHECKPOINT, "Read page %p from area %p-%p", curAddr, saved_area.addr, saved_area.endAddr);

        char flag;
        Utils::readAll(pmfd, &flag, sizeof(char));

        if (flag & Area::NO_PAGE) {
            // debuglogstdio(LCF_CHECKPOINT, "Page %p is no page", curAddr);
        }
        else if (flag & Area::ZERO_PAGE) {
            // debuglogstdio(LCF_CHECKPOINT, "Page %p is zero page", curAddr);
            memset(static_cast<void*>(curAddr), 0, 4096);
        }
        else if (flag & Area::BASE) {
            // debuglogstdio(LCF_CHECKPOINT, "Page %p is base page", curAddr);

            /* The memory page of the loading savestate is the same as the base
             * savestate. We must check if we actually need to read from the
             * base savestate or if the page already contains the correct values.
             */

            char parent_flag = parent_state.getPageFlag(curAddr);
            parent_state.skipPage(parent_flag);

            if (!(parent_flag & Area::BASE)) {
                /* Memory page has been modified between the two savestates.
                 * We must read from the base savestate.
                 */
                // debuglogstdio(LCF_CHECKPOINT, "Parent Page is not base page");
                char base_flag = base_state.getPageFlag(curAddr);
                MYASSERT(base_flag == Area::NONE);
                Utils::readAll(base_state.getPageFd(), static_cast<void*>(curAddr), 4096);
            }
            else {
                // debuglogstdio(LCF_CHECKPOINT, "Parent Page is base page");

                /* Gather the flag for the page map */
                lseek(fd, reinterpret_cast<off_t>(curAddr) / (4096/8), SEEK_SET);
                uint64_t page;
                Utils::readAll(fd, &page, 8);
                bool soft_dirty = page & (0x1ull << 55);

                //if (true) {
                if (soft_dirty) {
                    /* Memory page has been modified after parent state.
                     * We must read from the base savestate.
                     */
                    // debuglogstdio(LCF_CHECKPOINT, "Page is dirty");

                    char base_flag = base_state.getPageFlag(curAddr);
                    MYASSERT(base_flag == Area::NONE);
                    Utils::readAll(base_state.getPageFd(), static_cast<void*>(curAddr), 4096);
                }
            }
        }
        else {
            Utils::readAll(pfd, static_cast<void*>(curAddr), 4096);
        }
    }

    NATIVECALL(close(fd));

    /* Recover permission */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
    }
}


static void writeAllAreas(bool base)
{
    int pmfd, pfd;

    /* Because we may overwrite our parent state, we must save on a temp file
     * and rename it at the end. Again, we must not allocate any memory, so
     * we store the strings on the stack.
     */
    char temppagemappath[1024];
    char temppagespath[1024];

    if (base) {
        debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s", basepagespath);

        NATIVECALL(pmfd = creat(basepagemappath, 0644));
        NATIVECALL(pfd = creat(basepagespath, 0644));
    }
    else {
        strncpy(temppagemappath, pagemappath, 1023);
        strncpy(temppagespath, pagespath, 1023);

        strncat(temppagemappath, ".temp", 1023 - strlen(temppagemappath));
        strncat(temppagespath, ".temp", 1023 - strlen(temppagespath));

        debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s and %s", temppagemappath, temppagespath);

        unlink(temppagemappath);
        NATIVECALL(pmfd = creat(temppagemappath, 0644));

        unlink(temppagespath);
        NATIVECALL(pfd = creat(temppagespath, 0644));
    }

    MYASSERT(pmfd != -1)
    MYASSERT(pfd != -1)

    int spmfd;
    NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
    MYASSERT(spmfd != -1);

    int crfd;
    NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
    MYASSERT(crfd != -1);

    /* Saving the savestate header */
    StateHeader sh;
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED) {
            sh.pthread_ids[n] = thread->pthread_id;
            sh.tids[n++] = thread->tid;
        }
    }
    sh.thread_count = n;
    Utils::writeAll(pmfd, &sh, sizeof(sh));

    /* Load the parent savestate if any. */
    SaveState parent_state(parentpagemappath, parentpagespath);
    SaveState base_state(basepagemappath, basepagespath);

    /* Parse the content of /proc/self/maps into memory.
     * We don't allocate memory here, we are using our special allocated
     * memory section that won't be saved in the savestate.
     */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(0), ONE_MB);

    /* Remove write and add read flags from all memory areas we will be dumping */
    Area area;
    while (procSelfMaps.getNextArea(&area)) {
        if (!skipArea(&area)) {
            MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ) & ~PROT_WRITE) == 0)
        }
    }

    /* Dump all memory areas */
    procSelfMaps.reset();
    while (procSelfMaps.getNextArea(&area)) {
        if (skipArea(&area)) {
            area.properties |= Area::SKIP;
            Utils::writeAll(pmfd, &area, sizeof(area));
        }
        else {
            writeAnArea(pmfd, pfd, spmfd, area, parent_state, base_state);
        }
    }

    /* Add the last null (eof) area */
    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    Utils::writeAll(pmfd, &area, sizeof(area));

    /* Clear soft-dirty bits */
    Utils::writeAll(crfd, "4\n", 2);

    /* Recover area protection */
    procSelfMaps.reset();
    while (procSelfMaps.getNextArea(&area)) {
        if (!skipArea(&area)) {
            MYASSERT(mprotect(area.addr, area.size, area.prot) == 0)
        }
    }

    NATIVECALL(close(crfd));
    NATIVECALL(close(spmfd));

    /* Closing the savestate files */
    NATIVECALL(close(pmfd));
    NATIVECALL(close(pfd));

    /* Rename the savestate files */
    if (!base) {
        rename(temppagemappath, pagemappath);
        rename(temppagespath, pagespath);
    }
}

static void writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state, SaveState &base_state)
{
    //area.print("Save");

    /* Save the position of the first area page in the pages file */
    area.page_offset = lseek(pfd, 0, SEEK_CUR);

    /* Write the area struct */
    Utils::writeAll(pmfd, &area, sizeof(area));

    char* endAddr = static_cast<char*>(area.endAddr);
    for (char* curAddr = static_cast<char*>(area.addr); curAddr < endAddr; curAddr += 4096) {

        /* Gather the flag for the page map */
        lseek(spmfd, reinterpret_cast<off_t>(curAddr) / (4096/8), SEEK_SET);
        uint64_t page;
        Utils::readAll(spmfd, &page, 8);
        bool page_present = page & (0x1ull << 63);
        bool soft_dirty = page & (0x1ull << 55);

        /* Check if page is present */
        if (!page_present) {
            char flag = Area::NO_PAGE;
            Utils::writeAll(pmfd, &flag, sizeof(flag));
        }

        /* Check if page is zero */
        else if (Utils::isZeroPage(static_cast<void*>(curAddr))) {
            char flag = Area::ZERO_PAGE;
            Utils::writeAll(pmfd, &flag, sizeof(flag));
        }

        /* Check if page was not modified since last savestate */
        else if (!soft_dirty) {
            /* Copy the value of the parent savestate if any */
            if (parent_state) {
                char parent_flag = parent_state.getPageFlag(curAddr);

                if (parent_flag & Area::SKIP) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Non-skipped area with soft-dirty cleared and parent savestate skipped, wtf. Saving the page just in case.");
                    char flag = Area::NONE;
                    Utils::writeAll(pmfd, &flag, sizeof(flag));
                    Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
                }
                else if ((parent_flag & Area::NO_PAGE) || (parent_flag & Area::ZERO_PAGE) || (parent_flag & Area::BASE)) {
                    Utils::writeAll(pmfd, &parent_flag, sizeof(parent_flag));
                }
                else {
                    /* Parent state stores the memory page, we must store it too */
                    char flag = Area::NONE;
                    Utils::writeAll(pmfd, &flag, sizeof(flag));
                    Utils::writeAll(pfd, parent_state.getPage(parent_flag), 4096);
                }
            }
            else {
                // char base_flag = base_state.getPageFlag(curAddr);
                // char* base_page = base_state.getPage(base_flag);
                // if (0 != memcmp(base_page, curAddr, 4096)) {
                //     debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Non-dirty page mismatch!");
                // }
                // else {
                //     debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Non-dirty page match!");
                // }
                char flag = Area::BASE;
                Utils::writeAll(pmfd, &flag, sizeof(flag));
            }
        }
        else {
            char flag = Area::NONE;
            Utils::writeAll(pmfd, &flag, sizeof(flag));
            Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
        }
    }
}

}
