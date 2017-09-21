/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "detectsavefiles.h"
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include "../GlobalState.h"
#include "../inputs/jsdev.h"

namespace libtas {

/*** Helper functions ***/

static std::map<std::string,int> savefile_fds;

/*
 * Create an anonymous file with a copy of the content of the file using
 * memfd_create. Save the file descriptor into a map because we may
 * need it again if the game open the file again in read or write mode.
 * We don't actually close the file if the game ask for it, so we can keep
 * the content of the file in memory, and return the same fd if the game opens
 * the file again.
 */
static int get_memfd(const char* source, int flags)
{
    std::string sstr(source);
    int fd;

    /* Do we need to overwrite the content of the file ? */
    bool overwrite = ((flags & O_TRUNC) != 0);

    /*
     * If we already register the savefile, just return the file descriptor
     */
    if (savefile_fds.find(sstr) != savefile_fds.end()) {
        fd = savefile_fds[sstr];

        if (overwrite) {
            ftruncate(fd, 0);
        }
    }
    else {
        /* Create an anonymous file and store its file descriptor using the
         * recent memfd_create syscall.
         */
        fd = syscall(SYS_memfd_create, source, 0);
        savefile_fds[sstr] = fd;

        if (!overwrite) {
            /* Append the content of the file to the newly created memfile
             * if the file exists */
            struct stat filestat;
            int rv = stat(source, &filestat);

            if (rv == 0) {
                /* The file exists, copying the content to the stream */
                GlobalNative gn;

                FILE* f = fopen(source, "rb");

                if (f != nullptr) {
                    char tmp_buf[4096];
                    size_t s;
                    do {
                        s = fread(tmp_buf, 1, 4096, f);
                        write(fd, tmp_buf, s);
                    } while(s != 0);

                    fclose(f);
                }
            }
        }
    }

    /* If in append mode, seek to the end of the stream. If not, seek to the
     * beginning
     */
    if (flags & O_APPEND) {
        lseek(fd, 0, SEEK_END);
    }
    else {
        lseek(fd, 0, SEEK_SET);
    }

    return fd;
}

static bool isSaveFile(const char *file, int oflag)
{
    static bool inited = 0;
    if (!inited) {
        /*
         * Normally, we shouldn't have to clear the savefiles set,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the set
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
        savefile_fds.clear();
        inited = 1;
    }

    /* If the file has already been registered as a savefile, open the duplicate file,
     * even if the open is read-only.
     */
    std::string sstr(file);
    if (savefile_fds.find(sstr) != savefile_fds.end())
        return true;

    /* Check if file is writeable */
    if (!isWriteable(oflag))
        return false;

    return isSaveFile(file);
}

namespace orig {
    static int (*open) (const char *file, int oflag, ...);
    static int (*open64) (const char *file, int oflag, ...);
    static int (*openat) (int fd, const char *file, int oflag, ...);
    static int (*openat64) (int fd, const char *file, int oflag, ...);
    static int (*creat) (const char *file, mode_t mode);
    static int (*creat64) (const char *file, mode_t mode);
    static int (*close) (int fd);
}

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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return get_memfd(file, oflag);
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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return get_memfd(file, oflag);
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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return get_memfd(file, oflag);
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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return get_memfd(file, oflag);
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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
        return get_memfd(file, oflag);
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

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
        return get_memfd(file, oflag);
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

    /* We don't close the file if it is an opened savefile */
    for (auto& savefile_fd : savefile_fds) {
        if (savefile_fd.second == fd)
            return 0;
    }

    return orig::close(fd);
}

}

#endif
