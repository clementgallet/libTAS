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

#include "fileiowrappers.h"

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "logging.h"
#include "hook.h"
#include "../shared/SharedConfig.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include "GlobalState.h"

namespace libtas {

/*** Helper functions ***/

static std::map<std::string,std::pair<char*,size_t>> savefile_buffers;

/*
 * Create a memory stream with a copy of the content of the file using
 * open_memstream. Save the base pointer and size into a map because we may
 * need them again if the game open the file again in read or write mode.
 * If we already opened this file, don't copy the original content of the file
 * but the content of the old memory buffer.
 */
static FILE* get_memstream(const char* source, const char* modes)
{
    std::string sstr(source);
    FILE* memstream;

    /* Do we need to overwrite the content of the file ? */
    bool overwrite = (strstr(modes, "w") != nullptr);

    /*
     * If we already register the savefile, we must copy the content of the
     * old buffer to the new stream.
     */
    if (savefile_buffers.find(sstr) != savefile_buffers.end()) {

        /* Open a new memory stream using pointers to the previous memory buffer
         * and size. Pointers are not updated until fflush or fclose, so we
         * still have access to the old buffer.
         */
        memstream = open_memstream(&savefile_buffers[sstr].first, &savefile_buffers[sstr].second);

        if (!overwrite) {
            /* Append the content of the old buffer to the stream */
            fwrite(savefile_buffers[sstr].first, 1, savefile_buffers[sstr].second, memstream);
        }

        /* Free the old buffer */
        free(savefile_buffers[sstr].first);

    }
    else {
        /* Create an entry in our map */
        savefile_buffers[sstr].first = 0;
        savefile_buffers[sstr].second = 0;

        /* Open a new memory stream using pointers to these entries */
        memstream = open_memstream(&savefile_buffers[sstr].first, &savefile_buffers[sstr].second);

        if (!overwrite) {
            /* Append the content of the file to the stream if the file exists */
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
                        fwrite(tmp_buf, 1, s, memstream);
                    } while(s != 0);
                }
            }
        }
    }

    /* If not in append mode, seek to the beginning of the stream */
    if (strstr(modes, "a") == nullptr)
        fseek(memstream, 0, SEEK_SET);

    return memstream;
}

/* Check if the file open permission allows for write operation */
static bool isWriteable(const char *modes)
{
    if (strstr(modes, "w") || strstr(modes, "a") || strstr(modes, "+"))
        return true;
    return false;
}

/* Detect save files (excluding the writeable flag), basically if the file is regular */
static bool isSaveFile(const char *file)
{
    if (!shared_config.prevent_savefiles)
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
    if (! S_ISREG(filestat.st_mode))
        return false;

    /* Check if the file is a message queue, semaphore or shared memory object */
    if (S_TYPEISMQ(&filestat) || S_TYPEISSEM(&filestat) || S_TYPEISSHM(&filestat))
        return false;

    return true;
}

/* Specific savefile check for stdio and SDL open functions */
static bool isSaveFile(const char *file, const char *modes)
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
        savefile_buffers.clear();
        inited = 1;
    }

    /* If the file has already been registered as a savefile, open our memory
     * buffer, even if the open is read-only.
     */
    std::string sstr(file);
    if (savefile_buffers.find(sstr) != savefile_buffers.end())
        return true;

    /* If the file was not registered, check if the opening is writeable. */
    if (!isWriteable(modes))
        return false;

    /* Check the file is regular */
    return isSaveFile(file);
}


/*** SDL file IO ***/

namespace orig {
    static SDL_RWops *(*SDL_RWFromFile)(const char *file, const char *mode);
}

SDL_RWops *SDL_RWFromFile(const char *file, const char *mode)
{
    debuglog(LCF_FILEIO, __func__, " call with file ", file, " with mode ", mode);
    LINK_NAMESPACE_SDL2(SDL_RWFromFile);

    SDL_RWops* handle;

    if (!GlobalState::isOwnCode() && isSaveFile(file, mode)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected (not supported)");
    }
    NATIVECALL(handle = orig::SDL_RWFromFile(file, mode));

    return handle;
}

/*** stdio file IO ***/

namespace orig {
    static FILE *(*fopen) (const char *filename, const char *modes) = nullptr;
    static FILE *(*fopen64) (const char *filename, const char *modes) = nullptr;
    static int (*fclose) (FILE *stream) = nullptr;
}

FILE *fopen (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen, nullptr);

    if (GlobalState::isNative())
        return orig::fopen(filename, modes);

    /* iostream functions are banned inside this function, I'm not sure why.
     * This is the case for every open function.
     * Generating debug message using stdio.
     */
    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!GlobalState::isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = get_memstream(filename, modes);
    }
    else
        f = orig::fopen(filename, modes);

    return f;
}

FILE *fopen64 (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen64, nullptr);

    if (GlobalState::isNative())
        return orig::fopen64(filename, modes);

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!GlobalState::isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = get_memstream(filename, modes);
    }
    else
        f = orig::fopen64(filename, modes);

    return f;
}

int fclose (FILE *stream)
{
    LINK_NAMESPACE(fclose, nullptr);

    if (GlobalState::isNative())
        return orig::fclose(stream);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::fclose(stream);

    return rv;

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
    // static bool inited = 0;
    // if (!inited) {
        /*
         * Normally, we shouldn't have to clear the savefiles set,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the set
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
    //     savefiles.clear();
    //     inited = 1;
    // }

    /* If the file has already been registered as a savefile, open the duplicate file,
     * even if the open is read-only.
     */
    std::string sstr(file);
    if (savefile_buffers.find(sstr) != savefile_buffers.end())
        return true;

    /* Check if file is writeable */
    if (!isWriteable(oflag))
        return false;

    return isSaveFile(file);
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

    int fd;

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    fd = orig::open(file, oflag, mode);

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

    int fd;

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    fd = orig::open64(file, oflag, mode);

    return fd;
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

    int newfd;

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    newfd = orig::openat(fd, file, oflag, mode);

    return newfd;
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

    int newfd;

    if (!GlobalState::isOwnCode() && isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    newfd = orig::openat64(fd, file, oflag, mode);

    return newfd;
}

int creat (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat, nullptr);

    if (GlobalState::isNative())
        return orig::creat(file, mode);

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd;

    if (!GlobalState::isOwnCode()) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    fd = orig::creat(file, mode);

    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    LINK_NAMESPACE(creat64, nullptr);

    if (GlobalState::isNative())
        return orig::creat64(file, mode);

    debuglog(LCF_FILEIO, __func__, " call with file ", file);

    int fd;

    if (!GlobalState::isOwnCode()) {
        debuglogstdio(LCF_FILEIO | LCF_TODO, "  savefile detected");
    }

    fd = orig::creat64(file, mode);

    return fd;
}

int close (int fd)
{
    LINK_NAMESPACE(close, nullptr);

    if (GlobalState::isNative())
        return orig::close(fd);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int rv = orig::close(fd);

    return rv;
}

}

#endif
