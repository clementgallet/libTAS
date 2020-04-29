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
 */

#include "FileHandleList.h"

#include "FileHandle.h"
#include "../logging.h"
#include "../Utils.h"
#include "../inputs/evdev.h"
#include "../inputs/jsdev.h"

#include <cstdlib>
#include <forward_list>
#include <mutex>
#include <unistd.h> // lseek
#include <sys/ioctl.h>
#include <fcntl.h>

namespace libtas {

namespace FileHandleList {

/* This sounds really weird, but we are constructing our list of FileHandle
 * inside a function. This forces the list to be constructed when we need it
 * (a bit like the Singleton pattern). If we simply declare the list as a
 * static variable, then we will be using it before it has time to be
 * constructed (because some other libraries will initialize and open some files),
 * resulting in a crash.
 */
static std::forward_list<FileHandle>& getFileList() {
    static std::forward_list<FileHandle> filehandles;
    return filehandles;
}

/* Mutex to protect the file list */
static std::mutex& getFileListMutex() {
    static std::mutex mutex;
    return mutex;
}

void openFile(const char* file, int fd)
{
    if (fd < 0)
        return;

    std::lock_guard<std::mutex> lock(getFileListMutex());
    auto& filehandles = getFileList();

    /* Check if we already registered the file */
    for (const FileHandle &fh : filehandles) {
        if (fh.fds[0] == fd) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "Opened file descriptor %d was already registered!", fd);
            return;
        }
    }

    filehandles.emplace_front(file, fd);
}

void openFile(const char* file, FILE* f)
{
    if (!f)
        return;

    std::lock_guard<std::mutex> lock(getFileListMutex());
    auto& filehandles = getFileList();

    /* Check if we already registered the file */
    for (const FileHandle &fh : filehandles) {
        if (fh.stream == f) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "Opened file %p was already registered!", f);
            return;
        }
    }

    filehandles.emplace_front(file, f);
}

std::pair<int, int> createPipe(int flags) {
    int fds[2];
    if (pipe2(fds, flags) != 0)
        return std::make_pair(-1, -1);

    fcntl(fds[1], F_SETFL, O_NONBLOCK);
    std::lock_guard<std::mutex> lock(getFileListMutex());
    getFileList().emplace_front(fds);
    return std::make_pair(fds[0], fds[1]);
}

bool closeFile(int fd)
{
    if (fd < 0)
        return true;

    std::lock_guard<std::mutex> lock(getFileListMutex());
    auto& filehandles = getFileList();

    /* Check if we track the file */
    for (auto prev = filehandles.before_begin(), iter = filehandles.begin(); iter != filehandles.end(); prev = iter++) {
        if (iter->fds[0] == fd) {
            if (iter->tracked) {
                /* Just mark the file as closed, and tells to not close the file */
                iter->closed = true;
                return false;
            }
            else {
                if (!unref_evdev(iter->fds[0]) || !unref_jsdev(iter->fds[0])) {
                    return false;
                }
                if (iter->isPipe()) {
                    NATIVECALL(close(iter->fds[1]));
                }
                filehandles.erase_after(prev);
                return true;
            }
        }
    }

    debuglogstdio(LCF_FILEIO, "Unknown file descriptor %d", fd);
    return true;
}


void trackAllFiles()
{
    std::lock_guard<std::mutex> lock(getFileListMutex());

    for (FileHandle &fh : getFileList()) {
        debuglogstdio(LCF_FILEIO, "Track file %s (fd=%d,%d)", fh.fileName(), fh.fds[0], fh.fds[1]);
        fh.tracked = true;
        /* Save the file offset */
        if (!fh.closed) {
            if (fh.isPipe()) {
                /* By now all the threads are suspended, so we don't have to worry about
                 * racing to empty the pipe and possibly blocking.
                 */
                MYASSERT(ioctl(fh.fds[0], FIONREAD, &fh.pipeSize) == 0);
                debuglogstdio(LCF_FILEIO, "Save pipe size: %d", fh.pipeSize);
                if (fh.pipeSize > 0) {
                    std::free(fh.fileNameOrPipeContents);
                    fh.fileNameOrPipeContents = static_cast<char *>(std::malloc(fh.pipeSize));
                    Utils::readAll(fh.fds[0], fh.fileNameOrPipeContents, fh.pipeSize);
                }
            }
            else {
                if (fh.stream) {
                    fflush(fh.stream);
                    fh.fileOffset = ftell(fh.stream);
                }
                else {
                    fh.fileOffset = lseek(fh.fds[0], 0, SEEK_CUR);
                }
                debuglogstdio(LCF_FILEIO, "Save file offset: %d", fh.offset());
            }
        }
    }
}

void recoverAllFiles()
{
    std::lock_guard<std::mutex> lock(getFileListMutex());

    for (FileHandle &fh : getFileList()) {

        if (! fh.tracked) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "File %s (fd=%d,%d) not tracked when recovering", fh.fileName(), fh.fds[0], fh.fds[1]);
            continue;
        }

        /* Skip closed files */
        if (fh.closed) {
            continue;
        }

        int offset = fh.offset();
        ssize_t ret;
        if (fh.isPipe()) {
            /* Only recover if we have valid contents */
            if (!fh.fileNameOrPipeContents || fh.pipeSize < 0) {
                continue;
            }

            /* Empty the pipe */
            int pipesize;
            MYASSERT(ioctl(fh.fds[0], FIONREAD, &pipesize) == 0);
            if (pipesize != 0) {
                char* tmp = static_cast<char *>(std::malloc(pipesize));
                Utils::readAll(fh.fds[0], tmp, pipesize);
                std::free(tmp);
            }

            ret = Utils::writeAll(fh.fds[1], fh.fileNameOrPipeContents, fh.pipeSize);
            std::free(fh.fileNameOrPipeContents);
            fh.fileNameOrPipeContents = nullptr;
            fh.pipeSize = -1;
        }
        else {
            /* Only seek if we have a valid offset */
            if (fh.fileOffset == -1) {
                continue;
            }

            if (fh.stream) {
                ret = fseek(fh.stream, fh.fileOffset, SEEK_SET);
            }
            else {
                ret = lseek(fh.fds[0], fh.fileOffset, SEEK_SET);
            }
            fh.fileOffset = -1;
        }

        if (ret == -1) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "Error recovering %d bytes into file %s (fd=%d,%d)", offset, fh.fileName(), fh.fds[0], fh.fds[1]);
        }
        else {
            debuglogstdio(LCF_FILEIO, "Restore file %s (fd=%d,%d) offset to %d", fh.fileName(), fh.fds[0], fh.fds[1], offset);
        }
    }
}

void closeUntrackedFiles()
{
    std::lock_guard<std::mutex> lock(getFileListMutex());

    for (FileHandle &fh : getFileList()) {
        if (! fh.tracked) {
            if (fh.isPipe()) {
                NATIVECALL(close(fh.fds[0]));
                NATIVECALL(close(fh.fds[1]));
            }
            else {
                if (fh.stream)
                    NATIVECALL(fclose(fh.stream));
                else
                    NATIVECALL(close(fh.fds[0]));
            }
            /* We don't bother updating the file handle list, because it will be
             * replaced with the list from the loaded savestate.
             */
            debuglogstdio(LCF_FILEIO, "Close untracked file %s (fd=%d,%d)", fh.fileName(), fh.fds[0], fh.fds[1]);
        }
    }
}

}

}
