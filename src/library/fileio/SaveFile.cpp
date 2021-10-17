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

#include "SaveFile.h"

#include "../global.h" // shared_config
#include "../GlobalState.h"
#include "../logging.h"
#include "../Utils.h"
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <limits.h> //PATH_MAX

#ifdef __linux__
#include <sys/syscall.h>
#endif

namespace libtas {

SaveFile::SaveFile(const char *file) {
    /* Storing the canonicalized path so that we can compare paths. Only works
     * if the file actually exists. */
    char* canonfile = canonicalizeFile(file);
    if (! canonfile) {
        return;
    }
    filename = std::string(canonfile);
    free(canonfile);
    removed = false;
    closed = true;
    stream = nullptr;
    fd = 0;
}

SaveFile::~SaveFile() {
    /* Save back data into the file */
    if (shared_config.write_savefiles_on_exit && (fd != 0)) {
        debuglogstdio(LCF_FILEIO, "Save back into file %s", filename.c_str());
        GlobalNative gn;
        lseek(fd, 0, SEEK_SET);
        int file_fd = creat(filename.c_str(), 00777);
        if (file_fd >= 0) {
            char tmp_buf[4096];
            ssize_t s;
            do {
                s = Utils::readAll(fd, tmp_buf, 4096);
                Utils::writeAll(file_fd, tmp_buf, s);
            } while(s > 0);
            close(file_fd);
        }
    }

    if (stream) {
        NATIVECALL(fclose(stream));
    }
    else if (fd != 0) {
        NATIVECALL(close(fd));
    }
}

#define ISSLASH(c) ((c) == '/')

char* SaveFile::canonicalizeFile(const char *file)
{
    /* Code taken from gnulib canonicalize_filename_mode() function */
    char *rname, *dest;
    char const *start;
    char const *end;
    char const *rname_limit;
    char *absfile = nullptr;

    if (!file)
        return nullptr;

    if (file[0] == '\0')
        return nullptr;

    /* Convert relative to absolute path */
    if (!ISSLASH(file[0])) {
        absfile = static_cast<char*>(malloc (2*PATH_MAX));
        getcwd(absfile, PATH_MAX);
        size_t l = strnlen(absfile, PATH_MAX);
        absfile[l] = '/';
        strncpy(absfile+l+1, file, PATH_MAX);
        absfile[2*PATH_MAX-1] = '\0';

        start = absfile;
    }
    else {
        start = file;
    }

    rname = static_cast<char*>(malloc (PATH_MAX));
    rname_limit = rname + PATH_MAX;
    dest = rname;
    *dest++ = '/';

    for ( ; *start; start = end)
      {
        /* Skip sequence of multiple file name separators.  */
        while (ISSLASH (*start))
          ++start;

        /* Find end of component.  */
        for (end = start; *end && !ISSLASH (*end); ++end)
          /* Nothing.  */;

        if (end - start == 0)
          break;
        else if (end - start == 1 && start[0] == '.')
          /* nothing */;
        else if (end - start == 2 && start[0] == '.' && start[1] == '.')
          {
            /* Back up to previous component, ignore if at root already.  */
            if (dest > rname + 1)
              for (--dest; dest > rname && !ISSLASH (dest[-1]); --dest)
                continue;
          }
        else
          {

            if (!ISSLASH (dest[-1]))
              *dest++ = '/';

            if (dest + (end - start) >= rname_limit)
              {
                ptrdiff_t dest_offset = dest - rname;
                size_t new_size = rname_limit - rname;

                if (end - start + 1 > PATH_MAX)
                  new_size += end - start + 1;
                else
                  new_size += PATH_MAX;
                rname = static_cast<char*>(realloc (rname, new_size));
                rname_limit = rname + new_size;

                dest = rname + dest_offset;
              }

            dest = static_cast<char*>(memcpy (dest, start, end - start));
            dest += end - start;
            *dest = '\0';
          }
      }
    if (dest > rname + 1 && ISSLASH (dest[-1]))
      --dest;
    *dest = '\0';

    if (absfile)
        free(absfile);
    return rname;
}


bool SaveFile::isSameFile(const char *file)
{
    if (filename.empty())
        return false;

    /* Try comparing the canonilized paths */
    char* canonfile = canonicalizeFile(file);
    if (!canonfile)
        return false;

    const std::string filestr(canonfile);
    free(canonfile);
    return (filename.compare(filestr) == 0);
}

FILE* SaveFile::open(const char *modes) {

    if (filename.empty())
        return nullptr;

    /*
     * Create a memory stream with a copy of the content of the file using
     * open_memstream. Save the stream buffer and size.
     */

    closed = false;

    /* Check if file was removed by the game */
    if (removed) {
        if (strstr(modes, "r") != nullptr) {
            /* File was removed and opened in read-only mode */
            errno = ENOENT;
            closed = true;
            return nullptr;
        }
        else {
            /* File was removed and opened in write mode */
            this->open(O_RDWR); // creates a file descriptor
            NATIVECALL(stream = fdopen(fd, modes));
            setvbuf(stream, nullptr, _IONBF, 0);
            removed = false;
            return stream;
        }
    }

    if (stream == nullptr) {

        /* Open a new memory stream using pointers to these entries */
        this->open(O_RDWR); // creates a file descriptor
        NATIVECALL(stream = fdopen(fd, modes));
        setvbuf(stream, nullptr, _IONBF, 0);
        return stream;
    }

    /*
     * If we already opened the savefile:
     *   if opening in read, we seek at the beginning of the file
     *   if opening in append mode, we seek at the end of the file
     *   if opening in write mode, we seek at the beginning of the file and
     *   truncate the file descriptor.
     */
    if (strstr(modes, "w") != nullptr) {
        fseek(stream, 0, SEEK_SET);
        ftruncate(fd, 0);
        return stream;
    }
    else if (strstr(modes, "a") != nullptr) {
        fseek(stream, 0, SEEK_END);
        return stream;
    }

    fseek(stream, 0, SEEK_SET);
    return stream;

}

int SaveFile::open(int flags)
{
    if (filename.empty())
        return -1;

    /*
     * Create an anonymous file with a copy of the content of the file using
     * memfd_create, and save the file descriptor.
     * We don't actually close the file if the game ask for it, so we can keep
     * the content of the file in memory, and return the same fd if the game opens
     * the file again.
     */

    closed = false;

    /* Do we need to overwrite the content of the file ? */
    bool overwrite = ((flags & O_TRUNC) != 0);

    /* If we already register the savefile, just return the file descriptor. */
    if (fd != 0) {
        if (overwrite) {
            ftruncate(fd, 0);
        }
    }
    else if (removed) {
        if ((flags & O_ACCMODE) == O_RDONLY) {
            /* File was removed and opened in read-only mode */
            errno = ENOENT;
            closed = true;
            return -1;
        }
        else {
#ifdef __linux__
            /* File was removed and opened in write mode */
            fd = syscall(SYS_memfd_create, filename.c_str(), 0);
#else
            NATIVECALL(fd = ::open(filename.c_str(), flags));
#endif
        }
    }
    else {
#ifdef __linux__
        /* Create an anonymous file and store its file descriptor using memfd_create syscall. */
        fd = syscall(SYS_memfd_create, filename.c_str(), 0);

        if (!overwrite) {
            /* Append the content of the file to the newly created memfile
             * if the file exists */
            GlobalNative gn;
            struct stat filestat;
            int rv = stat(filename.c_str(), &filestat);

            if (rv == 0) {
                /* The file exists, copying the content to the stream */
                FILE* f = fopen(filename.c_str(), "rb");

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
#else
        NATIVECALL(fd = ::open(filename.c_str(), flags));
#endif
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

int SaveFile::closeFile()
{
    if (closed) {
        errno = EBADF;
        return -1;
    }

    closed = true;

    /* If the file wasn't removed, do nothing */
    if (!removed)
        return 0;

    if (stream) {
        NATIVECALL(fclose(stream)); // closes both the stream and fd
        stream = nullptr;
        fd = 0;
    }
    else if (fd != 0) {
        NATIVECALL(close(fd));
        fd = 0;
    }

    return 0;
}

int SaveFile::remove()
{
    if (removed) {
        errno = ENOENT;
        return -1;
    }

    removed = true;

    /* If the file wasn't closed, do nothing */
    if (!closed)
        return 0;

    if (stream) {
        NATIVECALL(fclose(stream)); // closes both the stream and fd
        stream = nullptr;
        fd = 0;
    }
    else if (fd != 0) {
        NATIVECALL(close(fd));
        fd = 0;
    }

    return 0;
}

}
