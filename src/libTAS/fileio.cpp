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
#include "logging.h"
#include "hook.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "backtrace.h"
#include "fcntl.h"

/*** SDL file IO ***/

SDL_RWops *(*SDL_RWFromFile_real)(const char *file, const char *mode);
SDL_RWops *(*SDL_RWFromFP_real)(FILE * fp, SDL_bool autoclose);

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
    SDL_RWops* handle = SDL_RWFromFile_real(file, mode);

    /* We replace the write callback with our own function */
    //if (handle)
    //    handle->write = dummyWrite;
    return handle;
}

SDL_RWops *SDL_RWFromFP(FILE * fp, SDL_bool autoclose)
{
    debuglog(LCF_FILEIO, __func__, " call");
    SDL_RWops* handle = SDL_RWFromFP_real(fp, autoclose);

    /* We replace the write callback with our own function */
    //if (handle)
    //    handle->write = dummyWrite;
    return handle;
}

void link_sdlfileio(void)
{
    LINK_SUFFIX_SDL2(SDL_RWFromFile);
    LINK_SUFFIX_SDL2(SDL_RWFromFP);
}

/*** stdio file IO ***/


FILE *(*fopen_real) (const char *filename, const char *modes) = nullptr;
FILE *(*fopen64_real) (const char *filename, const char *modes) = nullptr;
int (*fprintf_real) (FILE *stream, const char *format, ...) = nullptr;
int (*vfprintf_real) (FILE *s, const char *format, va_list arg) = nullptr;
int (*fputc_real) (int c, FILE *stream) = nullptr;
int (*putc_real) (int c, FILE *stream) = nullptr;
size_t (*fwrite_real) (const void *ptr, size_t size,
		      size_t n, FILE *s) = nullptr;

FILE *fopen (const char *filename, const char *modes)
{
    if (!fopen_real) {
        link_stdiofileio();
        if (!fopen_real) {
            printf("Failed to link fopen\n");
            return NULL;
        }
    }

    /* iostream functions are banned inside this function, I'm not sure why.
     * This is the case for every open function.
     * Generating debug message using stdio.
     */
    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);
    return fopen_real(filename, modes);
}

FILE *fopen64 (const char *filename, const char *modes)
{
    if (!fopen64_real) {
        link_stdiofileio();
        if (!fopen64_real) {
            printf("Failed to link fopen64\n");
            return NULL;
        }
    }

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    return fopen64_real(filename, modes);
}

int fprintf (FILE *stream, const char *format, ...)
{
    DEBUGLOGCALL(LCF_FILEIO);

    /* We cannot pass the arguments to fprintf_real. However, we
     * can build a va_list and pass it to vfprintf
     */
    va_list args;
    va_start(args, format);
    int ret = vfprintf_real(stream, format, args);
    va_end(args);
    return ret;
}

int vfprintf (FILE *s, const char *format, va_list arg)
{
    DEBUGLOGCALL(LCF_FILEIO);
    return vfprintf_real(s, format, arg);
}

int fputc (int c, FILE *stream)
{
    DEBUGLOGCALL(LCF_FILEIO);
    return fputc_real(c, stream);
}

int putc (int c, FILE *stream)
{
    DEBUGLOGCALL(LCF_FILEIO);
    return putc_real(c, stream);
}

size_t fwrite (const void *ptr, size_t size, size_t n, FILE *s)
{
    if (!fwrite_real) {
        link_stdiofileio();
        if (!fwrite_real) {
            printf("Failed to link fwrite\n");
            return 0;
        }
    }
    //DEBUGLOGCALL(LCF_FILEIO);
    //debuglogstdio(LCF_FILEIO, "%s call", __func__);
    return fwrite_real(ptr, size, n, s);
}

void link_stdiofileio(void)
{
    LINK_SUFFIX(fopen, nullptr);
    LINK_SUFFIX(fopen64, nullptr);
    LINK_SUFFIX(fprintf, nullptr);
    LINK_SUFFIX(vfprintf, nullptr);
    LINK_SUFFIX(fputc, nullptr);
    LINK_SUFFIX(putc, nullptr);
    LINK_SUFFIX(fwrite, nullptr);
}

int (*open_real) (const char *file, int oflag, ...);
int (*open64_real) (const char *file, int oflag, ...);
int (*openat_real) (int fd, const char *file, int oflag, ...);
int (*openat64_real) (int fd, const char *file, int oflag, ...);
int (*creat_real) (const char *file, mode_t mode);
int (*creat64_real) (const char *file, mode_t mode);
ssize_t (*write_real) (int fd, const void *buf, size_t n);
ssize_t (*pwrite_real) (int fd, const void *buf, size_t n, __off_t offset);
ssize_t (*pwrite64_real) (int fd, const void *buf, size_t n, __off64_t offset);

int open (const char *file, int oflag, ...)
{
    if (!open_real) {
        link_posixfileio();
        if (!open_real) {
            printf("Failed to link open\n");
            return -1;
        }
    }

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %d", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %d", __func__, oflag);

    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        int fd = open_real(file, oflag, mode);
        return fd;
    }
    else
    {
        int fd = open_real(file, oflag);
        return fd;
    }
}

int open64 (const char *file, int oflag, ...)
{
    if (!open64_real) {
        link_posixfileio();
        if (!open64_real) {
            printf("Failed to link open64\n");
            return -1;
        }
    }

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %d", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %d", __func__, oflag);

    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        int fd = open64_real(file, oflag, mode);
        return fd;
    }
    else
    {
        int fd = open64_real(file, oflag);
        return fd;
    }
}

int openat (int fd, const char *file, int oflag, ...)
{
    if (!openat_real) {
        link_posixfileio();
        if (!openat_real) {
            printf("Failed to link openat\n");
            return -1;
        }
    }

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %d", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %d", __func__, oflag);

    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        int newfd = openat_real(fd, file, oflag, mode);
        return newfd;
    }
    else
    {
        int newfd = openat_real(fd, file, oflag);
        return newfd;
    }
}

int openat64 (int fd, const char *file, int oflag, ...)
{
    if (!openat64_real) {
        link_posixfileio();
        if (!openat64_real) {
            printf("Failed to link openat64\n");
            return -1;
        }
    }
    
    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %d", __func__, file, oflag);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename and flag %d", __func__, oflag);

    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
    {
        va_list arg_list;
        mode_t mode;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);

        int newfd = openat64_real(fd, file, oflag, mode);
        return newfd;
    }
    else
    {
        int newfd = openat64_real(fd, file, oflag);
        return newfd;
    }
}

int creat (const char *file, mode_t mode)
{
    if (!creat_real) {
        link_posixfileio();
        if (!creat_real) {
            printf("Failed to link creat\n");
            return -1;
        }
    }

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd = creat_real(file, mode);
    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    if (!creat64_real) {
        link_posixfileio();
        if (!creat64_real) {
            printf("Failed to link creat64\n");
            return -1;
        }
    }

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd = creat64_real(file, mode);
    return fd;
}

ssize_t write (int fd, const void *buf, size_t n)
{
    if (!write_real) {
        link_posixfileio();
        if (!write_real) {
            printf("Failed to link write\n");
            return -1;
        }
    }
    DEBUGLOGCALL(LCF_FILEIO);
    return write_real(fd, buf, n);
}

ssize_t pwrite (int fd, const void *buf, size_t n, __off_t offset)
{
    if (!pwrite_real) return n;
    DEBUGLOGCALL(LCF_FILEIO);
    return pwrite_real(fd, buf, n, offset);
}

ssize_t pwrite64 (int fd, const void *buf, size_t n, __off64_t offset)
{
    if (!pwrite64_real) return n;
    DEBUGLOGCALL(LCF_FILEIO);
    return pwrite64_real(fd, buf, n, offset);
}

void link_posixfileio(void)
{
    LINK_SUFFIX(open, nullptr);
    LINK_SUFFIX(open64, nullptr);
    LINK_SUFFIX(openat, nullptr);
    LINK_SUFFIX(openat64, nullptr);
    LINK_SUFFIX(creat, nullptr);
    LINK_SUFFIX(creat64, nullptr);
    LINK_SUFFIX(write, nullptr);
    LINK_SUFFIX(pwrite, nullptr);
    LINK_SUFFIX(pwrite64, nullptr);
}

