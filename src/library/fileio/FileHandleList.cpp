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
 */

#include "FileHandleList.h"

#include "FileHandle.h"
#include "../logging.h"

#include <list>
#include <unistd.h> // lseek

namespace libtas {

namespace FileHandleList {

/* This sounds really weird, but we are constructing our list of FileHandle
 * inside a function. This forces the list to be constructed when we need it
 * (a bit like the Singleton pattern). If we simply declare the list as a
 * static variable, then we will be using it before it has time to be
 * constructed (because some other libraries will initialize and open some files),
 * resulting in a crash.
 */
static std::list<FileHandle>& getFileList() {
    static std::list<FileHandle> filehandles;
    return filehandles;
}

void openFile(const char* file, int fd)
{
    if (fd < 0)
        return;

    std::list<FileHandle>& filehandles = getFileList();

    /* Check if we already registered the file */
    for (const FileHandle &fh : filehandles) {
        if (fh.fd == fd) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "Opened file descriptor %d was already registered!", fd);
            return;
        }
    }

    FileHandle fh;
    fh.filename = file;
    fh.fd = fd;
    filehandles.push_back(fh);
}

bool closeFile(int fd)
{
    if (fd < 0)
        return true;

    std::list<FileHandle>& filehandles = getFileList();

    /* Check if we track the file */
    for (auto iter = filehandles.begin(); iter != filehandles.end(); iter++) {
        if (iter->fd == fd) {
            if (iter->tracked) {
                /* Just mark the file as closed, and tells to not close the file */
                iter->closed = true;
                return false;
            }
            else {
                filehandles.erase(iter);
                return true;
            }
        }
    }

    debuglogstdio(LCF_FILEIO, "Unknown file descriptor %d", fd);
    return true;
}


void trackAllFiles()
{
    std::list<FileHandle>& filehandles = getFileList();

    for (FileHandle &fh : filehandles) {
        debuglogstdio(LCF_FILEIO, "Track file %s (fd=%d)", fh.filename.c_str(), fh.fd);
        fh.tracked = true;
        /* Save the file offset */
        if (!fh.closed) {
            fh.offset = lseek(fh.fd, 0, SEEK_CUR);
            debuglogstdio(LCF_FILEIO, "Save file offset: %d", fh.offset);
        }
    }
}

void recoverAllFiles()
{
    std::list<FileHandle>& filehandles = getFileList();

    for (FileHandle &fh : filehandles) {

        if (! fh.tracked) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "File %s (fd=%d) not tracked when recovering", fh.filename.c_str(), fh.fd);
            continue;
        }

        /* Skip closed files */
        if (fh.closed) {
            continue;
        }

        /* Only seek if we have a valid offset */
        if (fh.offset == -1) {
            continue;
        }

        int ret = lseek(fh.fd, fh.offset, SEEK_SET);
        if (ret == -1) {
            debuglogstdio(LCF_FILEIO | LCF_ERROR, "Error seeking %d bytes into file %s (fd=%d)", fh.offset, fh.filename.c_str(), fh.fd);
        }
        else {
            debuglogstdio(LCF_FILEIO, "Restore file %s (fd=%d) offset to %d", fh.filename.c_str(), fh.fd, fh.offset);
        }
    }
}

void closeUntrackedFiles()
{
    std::list<FileHandle>& filehandles = getFileList();

    for (FileHandle &fh : filehandles) {
        if (! fh.tracked) {
            NATIVECALL(close(fh.fd));
            /* We don't bother updating the file handle list, because it will be
             * replaced with the list from the loaded savestate.
             */
            debuglogstdio(LCF_FILEIO, "Close untracked file %s (fd=%d)", fh.filename.c_str(), fh.fd);
        }
    }
}

}

}
