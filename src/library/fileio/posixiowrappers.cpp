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

#include "posixiowrappers.h"

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
#include "FileHandleList.h"
#include "../GlobalState.h"
#include "../inputs/jsdev.h"
#include "../inputs/evdev.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstdarg>
#include <cstring>

namespace libtas {

DEFINE_ORIG_POINTER(open)
DEFINE_ORIG_POINTER(open64)
DEFINE_ORIG_POINTER(openat)
DEFINE_ORIG_POINTER(openat64)
DEFINE_ORIG_POINTER(creat)
DEFINE_ORIG_POINTER(creat64)
DEFINE_ORIG_POINTER(close)
DEFINE_ORIG_POINTER(access)
DEFINE_ORIG_POINTER(__xstat)
DEFINE_ORIG_POINTER(__lxstat)
DEFINE_ORIG_POINTER(__fxstat)
DEFINE_ORIG_POINTER(__xstat64)
DEFINE_ORIG_POINTER(__lxstat64)
DEFINE_ORIG_POINTER(__fxstat64)
DEFINE_ORIG_POINTER(dup2)

int open (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open, nullptr);

    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::open(file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    /* Check if joystick device */
    if (is_jsdev(file) >= 0) {
        return open_jsdev(file, oflag);
    }

    if (is_evdev(file) >= 0) {
        return open_evdev(file, oflag);
    }

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::open(file, oflag, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int open64 (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open64, nullptr);

    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::open64(file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    /* Check if joystick device */
    if (is_jsdev(file) >= 0) {
        return open_jsdev(file, oflag);
    }

    if (is_evdev(file) >= 0) {
        return open_evdev(file, oflag);
    }

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::open64(file, oflag, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int openat (int dirfd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat, nullptr);

    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::openat(dirfd, file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::openat(dirfd, file, oflag, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int openat64 (int dirfd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat64, nullptr);

    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::openat64(dirfd, file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::openat64(dirfd, file, oflag, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int creat (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat, nullptr);

    if (GlobalState::isNative())
        return orig::creat(file, mode);

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    /* From creat() man page, creat() is just open() with flags
     * O_CREAT, O_WRONLY and O_TRUNC
     */
    int oflag = O_CREAT|O_WRONLY|O_TRUNC;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::creat(file, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat64, nullptr);

    if (GlobalState::isNative())
        return orig::creat64(file, mode);

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int oflag = O_CREAT|O_WRONLY|O_TRUNC;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    int fd = orig::creat64(file, mode);

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int close (int fd)
{
    LINK_NAMESPACE(close, nullptr);

    if (GlobalState::isNative())
        return orig::close(fd);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    close_jsdev(fd);
    close_evdev(fd);

    int ret = SaveFileList::closeSaveFile(fd);
    if (ret != 1)
        return ret;

    /* Check if we must actually close the file */
    bool doClose = FileHandleList::closeFile(fd);

    if (doClose)
        return orig::close(fd);

    return 0;
}

int access(const char *name, int type) throw()
{
    LINK_NAMESPACE(access, nullptr);

    if (GlobalState::isNative())
        return orig::access(name, type);

    debuglogstdio(LCF_FILEIO, "%s call with name %s", __func__, name);

    /* Check if joystick device */
    int joy = is_jsdev(name);
    if (joy == -1) joy = is_evdev(name);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }

    /* Check for savefile. */
    if (SaveFileList::getSaveFileFd(name) != 0) {
        if (SaveFileList::isSaveFileRemoved(name)) {
            errno = ENOENT;
            return -1;
        }
        else {
            return 0;
        }
    }

    return orig::access(name, type);
}

int __xstat(int ver, const char *path, struct stat *buf) throw()
{
    LINK_NAMESPACE(__xstat, nullptr);

    if (GlobalState::isNative())
        return orig::__xstat(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat(ver, fd, buf));
            return 0;
        }
    }

    return orig::__xstat(ver, path, buf);
}

int __xstat64(int ver, const char *path, struct stat64 *buf) throw()
{
    LINK_NAMESPACE(__xstat64, nullptr);

    if (GlobalState::isNative())
        return orig::__xstat64(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat64(ver, fd, buf));
            return 0;
        }
    }

    return orig::__xstat64(ver, path, buf);
}

int __lxstat(int ver, const char *path, struct stat *buf) throw()
{
    LINK_NAMESPACE(__lxstat, nullptr);

    if (GlobalState::isNative())
        return orig::__lxstat(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat(ver, fd, buf));
            return 0;
        }
    }

    return orig::__lxstat(ver, path, buf);
}

int __lxstat64(int ver, const char *path, struct stat64 *buf) throw()
{
    LINK_NAMESPACE(__lxstat64, nullptr);

    if (GlobalState::isNative())
        return orig::__lxstat64(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat64(ver, fd, buf));
            return 0;
        }
    }

    return orig::__lxstat64(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat *buf) throw()
{
    LINK_NAMESPACE(__fxstat, nullptr);

    if (GlobalState::isNative())
        return orig::__fxstat(ver, fd, buf);

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);
    return orig::__fxstat(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64 *buf) throw()
{
    LINK_NAMESPACE(__fxstat64, nullptr);

    if (GlobalState::isNative())
        return orig::__fxstat64(ver, fd, buf);

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);
    return orig::__fxstat64(ver, fd, buf);
}

int dup2 (int fd, int fd2) throw()
{
    debuglogstdio(LCF_FILEIO, "%s call: %d -> %d", __func__, fd2, fd);
    LINK_NAMESPACE(dup2, nullptr);

    if (fd2 == 2) {
        /* Prevent the game from redirecting stderr (2) to a file */
        return 2;
    }

    return orig::dup2(fd, fd2);
}

}
