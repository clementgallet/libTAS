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

#include "fileio.h"

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "logging.h"
#include "hook.h"
#include "../shared/Config.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <set>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include "ThreadState.h"

/*** SDL file IO ***/

namespace orig {
    static SDL_RWops *(*SDL_RWFromFile)(const char *file, const char *mode);
    static SDL_RWops *(*SDL_RWFromFP)(FILE * fp, SDL_bool autoclose);
}

size_t dummyWrite (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);
size_t dummyWrite (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num)
{
    debuglog(LCF_FILEIO, "Preventing writing ", num, " objects of size ", size);
    return num;
}

SDL_RWops *SDL_RWFromFile(const char *file, const char *mode)
{
    debuglog(LCF_FILEIO, __func__, " call with file ", file, " with mode ", mode);
    SDL_RWops* handle = orig::SDL_RWFromFile(file, mode);

    if (!config.prevent_savefiles) {
    /* We replace the write callback with our own function */
        if (handle)
            handle->write = dummyWrite;
    }
    return handle;
}

SDL_RWops *SDL_RWFromFP(FILE * fp, SDL_bool autoclose)
{
    debuglog(LCF_FILEIO, __func__, " call");
    SDL_RWops* handle = orig::SDL_RWFromFP(fp, autoclose);

    if (!config.prevent_savefiles) {
        /* We replace the write callback with our own function */
        if (handle)
            handle->write = dummyWrite;
    }
    return handle;
}

void link_sdlfileio(void)
{
    LINK_NAMESPACE_SDL2(SDL_RWFromFile);
    LINK_NAMESPACE_SDL2(SDL_RWFromFP);
}

/*** stdio file IO ***/

namespace orig {
    static FILE *(*fopen) (const char *filename, const char *modes) = nullptr;
    static FILE *(*fopen64) (const char *filename, const char *modes) = nullptr;
    static int (*fclose) (FILE *stream) = nullptr;
    static int (*fprintf) (FILE *stream, const char *format, ...) = nullptr;
    static int (*vfprintf) (FILE *s, const char *format, va_list arg) = nullptr;
    static int (*fputc) (int c, FILE *stream) = nullptr;
    static int (*putc) (int c, FILE *stream) = nullptr;
    static int (*putc_unlocked) (int c, FILE *stream);
    static int (*fputs) (const char *s, FILE *stream);
    static int (*fputs_unlocked) (const char *s, FILE *stream);
    static size_t (*fwrite) (const void *ptr, size_t size,
            size_t n, FILE *s) = nullptr;
}

static std::set<std::string> savefiles;

static std::string copyFile(const char* source)
{
    std::string dest(source);
    dest += ".libTAS";

    /*
     * If we already register the savefile, we already have
     * copied once, so we don't want to do it again.
     */
    if (savefiles.find(dest) != savefiles.end())
        return dest;

    threadState.setOwnCode(true);
    std::ifstream ss(source, std::ios::binary);
    std::ofstream ds(dest, std::ios::binary);

    if (!ss.fail()) {
        ds << ss.rdbuf();
    }

    savefiles.insert(dest);
    threadState.setOwnCode(false);
    return dest;
}

static bool isWriteable(const char *modes)
{
    if (strstr(modes, "w") || strstr(modes, "a") || strstr(modes, "+"))
        return true;
    return false;
}

static bool isSaveFile(const char *file)
{
    if (!config.prevent_savefiles)
        return false;

    if (!file)
        return false;

    static bool inited = 0;
    if (!inited) {
        /* 
         * Normally, we shouldn't have to clear the savefiles set,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the set
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
        savefiles.clear();
        inited = 1;
    }

    /* Check if file is a dev file */
    struct stat filestat;
    int rv = stat(file, &filestat);

    if (rv == -1) {
        /*
         * If the file does not exists,
         * we consider it as a savefile
         */
        if (errno == ENOENT)
            return true;

        /* For any other error, let's say no */
        return false;
    }

    /* Check if the file is a regular file */
    if (! S_ISREG(filestat.st_mode))
        return false;

    /* Check if the file is a message queue, semaphore or shared memory object */
    if (S_TYPEISMQ(&filestat) || S_TYPEISSEM(&filestat) || S_TYPEISSHM(&filestat))
        return false;

    return true;
}

static bool isSaveFile(const char *file, const char *modes)
{
    if (!isWriteable(modes))
        return false;

    return isSaveFile(file);
}

FILE *fopen (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen, nullptr);

    /* iostream functions are banned inside this function, I'm not sure why.
     * This is the case for every open function.
     * Generating debug message using stdio.
     */
    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!threadState.isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(filename);
        f = orig::fopen(newfile.c_str(), modes);
    }
    else
        f = orig::fopen(filename, modes);

    return f;
}

FILE *fopen64 (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen64, nullptr);

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!threadState.isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(filename);
        f = orig::fopen64(newfile.c_str(), modes);
    }
    else
        f = orig::fopen64(filename, modes);

    return f;
}

int fclose (FILE *stream)
{
    LINK_NAMESPACE(fclose, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::fclose(stream);

    return rv;

}

int fprintf (FILE *stream, const char *format, ...)
{
    LINK_NAMESPACE(fprintf, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    /* We cannot pass the arguments to fprintf_real. However, we
     * can build a va_list and pass it to vfprintf
     */
    va_list args;
    va_start(args, format);
    LINK_NAMESPACE(vfprintf, nullptr);
    int ret = orig::vfprintf(stream, format, args);
    va_end(args);
    return ret;
}

int vfprintf (FILE *s, const char *format, va_list arg)
{
    LINK_NAMESPACE(vfprintf, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::vfprintf(s, format, arg);
}

int fputc (int c, FILE *stream)
{
    LINK_NAMESPACE(fputc, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::fputc(c, stream);
}

int putc (int c, FILE *stream)
{
    LINK_NAMESPACE(putc, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::putc(c, stream);
}

int putc_unlocked (int c, FILE *stream)
{
    LINK_NAMESPACE(putc_unlocked, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::putc_unlocked(c, stream);
}

int fputs (const char *s, FILE *stream)
{
    LINK_NAMESPACE(fputs, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::fputs(s, stream);
}

int fputs_unlocked (const char *s, FILE *stream)
{
    LINK_NAMESPACE(fputs_unlocked, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::fputs_unlocked(s, stream);
}

size_t fwrite (const void *ptr, size_t size, size_t n, FILE *s)
{
    LINK_NAMESPACE(fwrite, nullptr);

    //DEBUGLOGCALL(LCF_FILEIO);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);
    return orig::fwrite(ptr, size, n, s);
}

namespace orig {
    static int (*open) (const char *file, int oflag, ...);
    static int (*open64) (const char *file, int oflag, ...);
    static int (*openat) (int fd, const char *file, int oflag, ...);
    static int (*openat64) (int fd, const char *file, int oflag, ...);
    static int (*creat) (const char *file, mode_t mode);
    static int (*creat64) (const char *file, mode_t mode);
    static int (*close) (int fd);
    static ssize_t (*write) (int fd, const void *buf, size_t n);
    static ssize_t (*pwrite) (int fd, const void *buf, size_t n, __off_t offset);
    static ssize_t (*pwrite64) (int fd, const void *buf, size_t n, __off64_t offset);
}

static bool isWriteable(int oflag)
{
    if ((oflag & 0x3) == O_RDONLY)
        return false;

    /*
     * This is a sort of hack to prevent considering new shared
     * memory files as a savefile, which are opened using O_CLOEXEC
     */
    if (oflag & O_CLOEXEC)
        return false;

    return true;
}

static bool isSaveFile(const char *file, int oflag)
{
    /* Check if file is writeable */
    if (!isWriteable(oflag))
        return false;

    return isSaveFile(file);
}

int open (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %o", __func__, oflag);

    int fd;
    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (!threadState.isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        fd = orig::open(newfile.c_str(), oflag, mode);
    }
    else
        fd = orig::open(file, oflag, mode);

    return fd;
}

int open64 (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open64, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %o", __func__, oflag);

    int fd;
    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (!threadState.isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        fd = orig::open64(newfile.c_str(), oflag, mode);
    }
    else
        fd = orig::open64(file, oflag, mode);

    return fd;
}

int openat (int fd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %o", __func__, oflag);

    int newfd;
    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (!threadState.isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        newfd = orig::openat(fd, newfile.c_str(), oflag, mode);
    }
    else
        newfd = orig::openat(fd, file, oflag, mode);

    return newfd;
}

int openat64 (int fd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat64, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %o", __func__, oflag);

    int newfd;
    mode_t mode;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (!threadState.isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        newfd = orig::openat64(fd, newfile.c_str(), oflag, mode);
    }
    else
        newfd = orig::openat64(fd, file, oflag, mode);

    return newfd;
}

int creat (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat, nullptr);
    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd;

    if (!threadState.isOwnCode() && file) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        fd = orig::creat(newfile.c_str(), mode);
    }
    else
        fd = orig::creat(file, mode);

    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat64, nullptr);
    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd;

    if (!threadState.isOwnCode() && file) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        std::string newfile = copyFile(file);
        fd = orig::creat64(newfile.c_str(), mode);
    }
    else
        fd = orig::creat64(file, mode);

    return fd;
}

int close (int fd)
{
    LINK_NAMESPACE(close, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::close(fd);

    return rv;
}

ssize_t write (int fd, const void *buf, size_t n)
{
    LINK_NAMESPACE(write, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::write(fd, buf, n);
}

ssize_t pwrite (int fd, const void *buf, size_t n, __off_t offset)
{
    LINK_NAMESPACE(pwrite, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::pwrite(fd, buf, n, offset);
}

ssize_t pwrite64 (int fd, const void *buf, size_t n, __off64_t offset)
{
    LINK_NAMESPACE(pwrite64, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    return orig::pwrite64(fd, buf, n, offset);
}

#else

void link_sdlfileio(void) {}

#endif

