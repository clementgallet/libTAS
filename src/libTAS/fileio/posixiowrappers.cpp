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

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
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

    const char* jsdevstr = "/dev/input/js";
    if ((strlen(file) > strlen(jsdevstr)) && (strncmp(jsdevstr, file, strlen(jsdevstr)) == 0)) {
        debuglogstdio(LCF_FILEIO | LCF_JOYSTICK, "  joystick device detected");
        return open_jsdev(file, oflag);
    }

    const char* evdevstr = "/dev/input/event";
    if ((strlen(file) > strlen(evdevstr)) && (strncmp(evdevstr, file, strlen(evdevstr)) == 0)) {
        debuglogstdio(LCF_FILEIO | LCF_JOYSTICK, "  event device detected");
        return open_evdev(file, oflag);
    }

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    return orig::open(file, oflag, mode);
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

    const char* jsdevstr = "/dev/input/js";
    if ((strlen(file) > strlen(jsdevstr)) && (strncmp(jsdevstr, file, strlen(jsdevstr)) == 0)) {
        debuglogstdio(LCF_FILEIO | LCF_JOYSTICK, "  joystick device detected");
        return open_jsdev(file, oflag);
    }

    const char* evdevstr = "/dev/input/event";
    if ((strlen(file) > strlen(evdevstr)) && (strncmp(evdevstr, file, strlen(evdevstr)) == 0)) {
        debuglogstdio(LCF_FILEIO | LCF_JOYSTICK, "  event device detected");
        return open_evdev(file, oflag);
    }

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    return orig::open64(file, oflag, mode);
}

int openat (int fd, const char *file, int oflag, ...)
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
        return orig::openat(fd, file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    return orig::openat(fd, file, oflag, mode);
}

int openat64 (int fd, const char *file, int oflag, ...)
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
        return orig::openat64(fd, file, oflag, mode);

    debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(file, oflag);
    }

    return orig::openat64(fd, file, oflag, mode);
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

    return orig::creat(file, mode);
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

    return orig::creat64(file, mode);
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

    return orig::close(fd);
}

int access(const char *name, int type) throw()
{
    LINK_NAMESPACE(access, nullptr);

    if (GlobalState::isNative())
        return orig::access(name, type);

    debuglogstdio(LCF_FILEIO, "%s call with name %s", __func__, name);

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

    /* Check if savefile. */
    if (SaveFileList::getSaveFileFd(path) != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            debuglogstdio(LCF_FILEIO | LCF_TODO, "    stat struct is not filled!");
            return 0;
        }
    }

    return orig::__xstat(ver, path, buf);
}

int __lxstat(int ver, const char *path, struct stat *buf) throw()
{
    LINK_NAMESPACE(__lxstat, nullptr);

    if (GlobalState::isNative())
        return orig::__lxstat(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    /* Check if savefile. */
    if (SaveFileList::getSaveFileFd(path) != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            debuglogstdio(LCF_FILEIO | LCF_TODO, "    stat struct is not filled!");
            return 0;
        }
    }

    return orig::__lxstat(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat *buf) throw()
{
    LINK_NAMESPACE(__fxstat, nullptr);

    if (GlobalState::isNative())
        return orig::__fxstat(ver, fd, buf);

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);
    return orig::__fxstat(ver, fd, buf);
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

#endif
