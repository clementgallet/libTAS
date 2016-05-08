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
#include <iostream>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include <errno.h>

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

static std::map<FILE*, std::string> stdio_savefiles;

static bool isWriteable(const char *modes)
{
    if ((strcmp(modes, "r") == 0) || (strcmp(modes, "rb") == 0))
        return false;
    return true;
}

static bool isSaveFile(const char *file)
{
    if (!config.prevent_savefiles)
        return false;

    if (!file)
        return false;

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
    if (S_ISREG(filestat.st_mode))
        return true;

    return false;
}

static bool isSaveFile(const char *file, const char *modes)
{
    if (!isWriteable(modes))
        return false;

    static bool inited = 0;
    if (!inited) {
        /* 
         * Normally, we shouldn't have to clear the posix_savefiles map,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the map
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
        stdio_savefiles.clear();
        inited = 1;
    }

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

    FILE* f = orig::fopen(filename, modes);

    if (isSaveFile(filename, modes))
        stdio_savefiles[f] = std::string(filename); // non NULL file is tested in isSaveFile()

    return f;
}

FILE *fopen64 (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen64, nullptr);

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f = orig::fopen64(filename, modes);

    if (isSaveFile(filename, modes))
        stdio_savefiles[f] = std::string(filename); // non NULL file is tested in isSaveFile()

    return f;
}

int fclose (FILE *stream)
{
    LINK_NAMESPACE(fclose, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::fclose(stream);

    if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
        debuglog(LCF_FILEIO, "  close savefile ", stdio_savefiles[stream]);
        stdio_savefiles.erase(stream);
    }

    return rv;

}

int fprintf (FILE *stream, const char *format, ...)
{
    LINK_NAMESPACE(fprintf, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);

            /* 
             * We still have to compute the number of characters that the
             * game think have been written. Using vsnprintf for this.
             *
             * We don't need to create a buffer containing all written characters
             * to get access to that number, so only using a one-length array
             */
            char c;

            va_list args;
            va_start(args, format);
            int ret = vsnprintf(&c, 1, format, args);
            va_end(args);

            return ret;
        }
    }

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

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(s) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[s]);

            /* 
             * We still have to compute the number of characters that the
             * game think have been written. Using vsnprintf for this.
             *
             * We don't need to create a buffer containing all written characters
             * to get access to that number, so only using a one-length array
             */
            char c;

            int ret = vsnprintf(&c, 1, format, arg);
            return ret;
        }
    }

    return orig::vfprintf(s, format, arg);
}

int fputc (int c, FILE *stream)
{
    LINK_NAMESPACE(fputc, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);
            return c;
        }
    }

    return orig::fputc(c, stream);
}

int putc (int c, FILE *stream)
{
    LINK_NAMESPACE(putc, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);
            return c;
        }
    }

    return orig::putc(c, stream);
}

int putc_unlocked (int c, FILE *stream)
{
    LINK_NAMESPACE(putc_unlocked, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);
            return c;
        }
    }

    return orig::putc_unlocked(c, stream);
}

int fputs (const char *s, FILE *stream)
{
    LINK_NAMESPACE(fputs, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);
            return 0;
        }
    }

    return orig::fputs(s, stream);
}

int fputs_unlocked (const char *s, FILE *stream)
{
    LINK_NAMESPACE(fputs_unlocked, nullptr);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(stream) != stdio_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[stream]);
            return 0;
        }
    }

    return orig::fputs_unlocked(s, stream);
}

size_t fwrite (const void *ptr, size_t size, size_t n, FILE *s)
{
    LINK_NAMESPACE(fwrite, nullptr);

    if (config.prevent_savefiles) {
        if (stdio_savefiles.find(s) != stdio_savefiles.end()) {
            //debuglog(LCF_FILEIO, "  prevent write to ", stdio_savefiles[s]);
            if (size == 0)
                return 0;
            return n;
        }
    }

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

static std::map<int, std::string> posix_savefiles;

static bool isWriteable(int oflag)
{
    if ((oflag & 0x3) == O_RDONLY)
        return false;
    return true;
}

static bool isSaveFile(const char *file, int oflag)
{
    /* Check if file is writeable */
    if (!isWriteable(oflag))
        return false;

    static bool inited = 0;
    if (!inited) {
        /* 
         * Normally, we shouldn't have to clear the posix_savefiles map,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the map
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
        posix_savefiles.clear();
        inited = 1;
    }

    return isSaveFile(file);
}

int open (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %X", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %X", __func__, oflag);

    int fd;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        fd = orig::open(file, oflag, mode);
    }
    else
    {
        fd = orig::open(file, oflag);
    }

    if (isSaveFile(file, oflag))
        posix_savefiles[fd] = std::string(file); // non NULL file is tested in isSaveFile()

    return fd;
}

int open64 (const char *file, int oflag, ...)
{
    LINK_NAMESPACE(open64, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %X", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %X", __func__, oflag);

    int fd;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        fd = orig::open64(file, oflag, mode);
    }
    else
    {
        fd = orig::open64(file, oflag);
    }

    if (isSaveFile(file, oflag))
        posix_savefiles[fd] = std::string(file); // non NULL file is tested in isSaveFile()

    return fd;
}

int openat (int fd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %X", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %X", __func__, oflag);

    int newfd;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        newfd = orig::openat(fd, file, oflag, mode);
    }
    else
    {
        newfd = orig::openat(fd, file, oflag);
    }

    if (isSaveFile(file, oflag))
        posix_savefiles[newfd] = std::string(file); // non NULL file is tested in isSaveFile()

    return newfd;
}

int openat64 (int fd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE(openat64, nullptr);
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %X", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %X", __func__, oflag);

    int newfd;
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        newfd = orig::openat64(fd, file, oflag, mode);
    }
    else
    {
        newfd = orig::openat64(fd, file, oflag);
    }

    if (isSaveFile(file, oflag))
        posix_savefiles[newfd] = std::string(file); // non NULL file is tested in isSaveFile()

    return newfd;
}

int creat (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat, nullptr);
    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd = orig::creat(file, mode);

    if (file)
        posix_savefiles[fd] = std::string(file);

    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat64, nullptr);
    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd = orig::creat64(file, mode);

    if (file)
        posix_savefiles[fd] = std::string(file);

    return fd;
}

int close (int fd)
{
    LINK_NAMESPACE(close, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::close(fd);

    if (posix_savefiles.find(fd) != posix_savefiles.end()) {
        debuglog(LCF_FILEIO, "  close savefile ", posix_savefiles[fd]);
        posix_savefiles.erase(fd);
    }

    return rv;
}

ssize_t write (int fd, const void *buf, size_t n)
{
    LINK_NAMESPACE(write, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (posix_savefiles.find(fd) != posix_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", posix_savefiles[fd]);
            return n;
        }
    }
    return orig::write(fd, buf, n);
}

ssize_t pwrite (int fd, const void *buf, size_t n, __off_t offset)
{
    LINK_NAMESPACE(pwrite, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (posix_savefiles.find(fd) != posix_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", posix_savefiles[fd]);
            return n;
        }
    }
    return orig::pwrite(fd, buf, n, offset);
}

ssize_t pwrite64 (int fd, const void *buf, size_t n, __off64_t offset)
{
    LINK_NAMESPACE(pwrite64, nullptr);
    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (config.prevent_savefiles) {
        if (posix_savefiles.find(fd) != posix_savefiles.end()) {
            debuglog(LCF_FILEIO, "  prevent write to ", posix_savefiles[fd]);
            return n;
        }
    }
    return orig::pwrite64(fd, buf, n, offset);
}

#else

void link_sdlfileio(void) {}

#endif

