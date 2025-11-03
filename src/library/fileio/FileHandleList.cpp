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
 */

#include "FileHandleList.h"
#include "FileHandle.h"
#include "FileDescriptorManip.h"
#include "SaveFile.h"
#include "SaveFileList.h"

#include "logging.h"
#include "Profiler.h"
#include "Utils.h"
#include "GlobalState.h"
#include "global.h"
#ifdef __linux__
#include "inputs/evdev.h"
#include "inputs/jsdev.h"
#endif

#include <cstdlib>
#include <forward_list>
#include <mutex>
#include <unistd.h> // lseek
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#ifdef __linux__
#include <sys/syscall.h>
#endif

namespace libtas {

namespace FileHandleList {

/* Constructing this (as well as other elements elsewhere as local-scope static
 * pointer. This forces the list to be constructed when we need it
 * (a bit like the Singleton pattern). If we simply declare the list as a
 * static variable, then we will be using it before it has time to be
 * constructed (because some other libraries will initialize and open some files),
 * resulting in a crash. Also, we allocate it dynamically and never free it, so
 * that it has a chance to survive every other game code that may use it.
 */
std::forward_list<FileHandle>& getFileList() {
    static std::forward_list<FileHandle>* filehandles = new std::forward_list<FileHandle>;
    return *filehandles;
}

std::pair<int, int> createPipe(int flags) {
    int fds[2];
#ifdef __linux__
    if (pipe2(fds, flags) != 0)
#else
    if (pipe(fds) != 0)
#endif
        return std::make_pair(-1, -1);

    fcntl(fds[1], F_SETFL, O_NONBLOCK);
    getFileList().emplace_front("", fds);
    return std::make_pair(fds[0], fds[1]);
}

int fdFromFile(const char* file)
{
    auto& filehandles = getFileList();

    for (const FileHandle &fh : filehandles) {
        if (fh.type == FileHandle::FILE_PIPE)
            continue;
        if (0 == strcmp(fh.fileName, file)) {
            return fh.fds[0];
        }
    }
    return -1;
}

const FileHandle& fileHandleFromFd(int fd)
{
    auto& filehandles = getFileList();

    static FileHandle fh_zero;

    for (const FileHandle &fh : filehandles) {
        if (fh.fds[0] == fd)
            return fh;
        if ((fh.type == FileHandle::FILE_PIPE) && (fh.fds[1] == fd))
            return fh;
    }
    return fh_zero;
}

void updateAllFiles()
{
    PROFILE_SCOPE("File Handles", PROFILER_INFO_FRAME);

    GlobalNative gn;

    auto& filehandles = getFileList();
    filehandles.clear();

    struct dirent *dp;

    DIR *dir = opendir("/proc/self/fd/");
    int dir_fd = dirfd(dir);
    
    while ((dp = readdir(dir))) {
        if (dp->d_type != DT_LNK)
            continue;

        int fd = std::atoi(dp->d_name);
        
        /* Skip own dir file descriptor */
        if (fd == dir_fd)
            continue;

        /* Get symlink */
        char buf[1024] = {};
        ssize_t buf_size = readlinkat(dir_fd, dp->d_name, buf, 1024);
        if (buf_size == -1) {
            LOG(LL_WARN, LCF_FILEIO, "Cound not get symlink to file fd %d", fd);
        }
        else if (buf_size == 1024) {
            /* Truncation occured */
            buf[1023] = '\0';
            LOG(LL_WARN, LCF_FILEIO, "Adding file with fd %d to file handle list failed because symlink was truncated: %s", fd, buf);
        }
        else {
            if (0 == strncmp(buf, "pipe:", 5)) {
                /* Check if the pipe was already added from the other file descriptor */
                bool pipe_already_pushed = false;

                for (FileHandle &fh : filehandles) {
                    if ((fh.type == FileHandle::FILE_PIPE) && (0 == strcmp(fh.fileName, buf))) {
                        /* Add the second fd to the pipe */
                        if (fh.fds[0] == -1)
                            fh.fds[0] = fd;
                        else if (fh.fds[1] == -1)
                            fh.fds[1] = fd;
                        else
                            LOG(LL_ERROR, LCF_FILEIO, "Pipe %s with fd %d already met a complete pipe (fd=%d,%d)", buf, fd, fh.fds[0], fh.fds[1]);

                        pipe_already_pushed = true;
                        break;
                    }
                }
                
                if (!pipe_already_pushed) {
                    /* Find which end of the pipe are we processing */
                    bool is_write_end = (0 == faccessat(dir_fd, dp->d_name, W_OK, AT_SYMLINK_NOFOLLOW));
                    
                    /* We append the pipe with this fd, and later will fill the other fd. */
                    int fds[2] = {-1, -1};
                    filehandles.emplace_front(buf, fds);
                    if (is_write_end)
                        filehandles.front().fds[1] = fd;
                    else
                        filehandles.front().fds[0] = fd;
                }
            }
            else if (0 == strncmp(buf, "socket:", 7))
                filehandles.emplace_front(buf, fd, FileHandle::FILE_SOCKET);
            else if (0 == strncmp(buf, "/dev/", 5))
                filehandles.emplace_front(buf, fd, FileHandle::FILE_DEVICE);
            else if (0 == strncmp(buf, "/memfd:", 7))
                filehandles.emplace_front(buf, fd, FileHandle::FILE_MEMFD);
            else if (0 == strncmp(buf, "/dmabuf:", 8))
                filehandles.emplace_front(buf, fd, FileHandle::FILE_SPECIAL);
            else if (buf[0] == '/')
                filehandles.emplace_front(buf, fd, FileHandle::FILE_REGULAR);
            else
                filehandles.emplace_front(buf, fd, FileHandle::FILE_SPECIAL);
        }
    }
    closedir(dir);
}

void trackAllFiles()
{
    updateAllFiles();

    auto& filehandles = getFileList();
    
    for (FileHandle &fh : filehandles) {
        trackFile(fh);
    }
}

void trackFile(FileHandle &fh)
{

    /* Save the file offset */
    if (fh.type == FileHandle::FILE_PIPE) {
        /* By now all the threads are suspended, so we don't have to worry about
         * racing to empty the pipe and possibly blocking.
         */
        LOG(LL_DEBUG, LCF_FILEIO, "Save pipe content (fd=%d,%d)", fh.fds[0], fh.fds[1]);

        int pipeSize;
        MYASSERT(ioctl(fh.fds[0], FIONREAD, &pipeSize) == 0);
        LOG(LL_DEBUG, LCF_FILEIO, "Save pipe size: %d", pipeSize);
        fh.size = pipeSize;
        if (fh.size > 0) {
            std::free(fh.pipeContents);
            fh.pipeContents = static_cast<char *>(std::malloc(fh.size));
            Utils::readAll(fh.fds[0], fh.pipeContents, fh.size);
        }
    }
    else if (fh.needsTracking()) {
        LOG(LL_DEBUG, LCF_FILEIO, "Track file %s (fd=%d)", fh.fileName, fh.fds[0]);

        fdatasync(fh.fds[0]);
        fh.fileOffset = lseek(fh.fds[0], 0, SEEK_CUR);
        fh.size = lseek(fh.fds[0], 0, SEEK_END);
        lseek(fh.fds[0], fh.fileOffset, SEEK_SET);
        LOG(LL_DEBUG, LCF_FILEIO, "Save file offset %jd and size %jd", fh.fileOffset, fh.size);
    }
}

void recoverFileOffsets()
{
    for (FileHandle &fh : getFileList()) {

        if (!fh.needsTracking())
            continue;

        /* Only seek if we have a valid offset */
        if (fh.fileOffset == -1) {
            continue;
        }

        off_t current_size = lseek(fh.fds[0], 0, SEEK_END);
        ssize_t ret = lseek(fh.fds[0], fh.fileOffset, SEEK_SET);

        if (current_size != fh.size) {
            LOG(LL_WARN, LCF_FILEIO, "File %s (fd=%d) changed size from %jd to %jd", fh.fileName, fh.fds[0], fh.size, current_size);
        }

        if (ret == -1) {
            LOG(LL_ERROR, LCF_FILEIO, "Error seeking %jd bytes into file %s (fd=%d)", fh.fileOffset, fh.fileName, fh.fds[0]);
        }
        else {
            LOG(LL_DEBUG, LCF_FILEIO, "Restore file offset %s (fd=%d) to %jd", fh.fileName, fh.fds[0], fh.fileOffset);
        }

        fh.fileOffset = -1;
    }
}

void recoverPipeContents()
{
    for (FileHandle &fh : getFileList()) {
        if (!(fh.type == FileHandle::FILE_PIPE))
            continue;

        /* Only recover if we have valid contents */
        if (!fh.pipeContents || fh.size < 0) {
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

        ssize_t ret = Utils::writeAll(fh.fds[1], fh.pipeContents, fh.size);

        if (ret == -1) {
            LOG(LL_ERROR, LCF_FILEIO, "Error recovering %jd bytes into pipe (fd=%d,%d)", fh.size, fh.fds[0], fh.fds[1]);
        }
        else {
            LOG(LL_DEBUG, LCF_FILEIO, "Restore pipe (fd=%d,%d) size to %jd", fh.fds[0], fh.fds[1], fh.size);
        }

        std::free(fh.pipeContents);
        fh.pipeContents = nullptr;
        fh.size = -1;
    }
}

void syncFileDescriptors()
{
    /* Browse all possible values of fd. We don't go past our reserve value,
     * which contains the fds for the state loading procedure */ 
    for (int fd = 3; fd < FileDescriptorManip::reserveState(); fd++) {
        /* Query for a registered file handle (excluding pipes) */
        const FileHandle& fh = FileHandleList::fileHandleFromFd(fd);
        bool fdRegistered = (fh.fds[0] == fd) || (fh.fds[1] == fd);

        /* Look at existing file descriptor and get symlink */
        char fd_str[25];
        sprintf(fd_str, "/proc/self/fd/%d", fd);

        char buf[1024] = {};
        ssize_t buf_size = readlink(fd_str, buf, 1024);
        bool fdOpened = false;

        if (buf_size != -1) {
            if (buf_size == 1024) {
                /* Truncation occured */
                buf[1023] = '\0';
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Symlink of file fd %d was truncated: %s", fd, buf);
            }

            fdOpened = true;
        }

        /* Handle all cases of fd */
        if (!fdRegistered && !fdOpened) {
            continue;
        }
        if (fdRegistered && fdOpened) {
            /* Check for matching path */
            if (0 != strcmp(fh.fileName, buf)) {
                LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "File descriptor %d is currently linked to %s, but was linked to %s in savestate", fd, buf, fh.fileName);
                close(fd);
                fdOpened = false;
            }
        }
        if (!fdRegistered && fdOpened) {
            /* The file descriptor must be closed, because it is not present in
             * the savestate */
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "File %s with fd %d should not be present after loading the state, so it is closed", buf, fd);
            close(fd);
        }
        if (fdRegistered && !fdOpened) {
            /* The file descriptor must be opened, because it is present in
             * the savestate */
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "File %s with fd %d should be opened", fh.fileName, fd);

            /* We must open the file with the same fd as the one saved in the state */
            int next_fd = FileDescriptorManip::enforceNext(fd);
            
            if (next_fd == -1) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because fd manipulation failed", fd);
                continue;
            }

            if (next_fd > fd) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because we went past it after calling dup(), which returned %d", fd, next_fd);
                continue;
            }

            if (next_fd != fd) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because it is already opened", fd);
                continue;
            }

            /* Recreate a memfd if savefile */
            const SaveFile* sf = SaveFileList::getSaveFile(fd);
            if (sf) {
                int new_fd = syscall(SYS_memfd_create, sf->filename.c_str(), 0);
                if (new_fd < 0) {
                    LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "Could not create memfd");
                    continue;                    
                }
                if (new_fd != fd) {
                    LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Recreate fd was supposed to be %d but is instead %d...", fd, new_fd);
                    close(new_fd);
                    continue;
                }
            }
            else if (fh.type == FileHandle::FILE_REGULAR) {
                /* TODO: We assume that the non-savefile file is read-only for now */
                int new_fd = open(fh.fileName, O_RDONLY);
                if (new_fd < 0) {
                    LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "Could not open file %s", fh.fileName);
                    continue;                    
                }
                if (new_fd != fd) {
                    LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Recreate fd was supposed to be %d but is instead %d...", fd, new_fd);
                    close(new_fd);
                    continue;
                }
            }
            else {
                LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "Non-standard file %s is supposed to be opened after state loaded", fh.fileName);
            }
        }
    }

    FileDescriptorManip::closeAll();
}

}

}
