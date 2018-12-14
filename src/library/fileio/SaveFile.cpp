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

#include "SaveFile.h"

#include "../global.h" // shared_config
#include "../GlobalState.h"
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <sys/syscall.h>
#include <unistd.h>

namespace libtas {

SaveFile::SaveFile(const char *file) {
    filename = std::string(file);
    removed = false;
    closed = true;
    stream = nullptr;
    fd = 0;
}

SaveFile::~SaveFile() {
    if (stream) {
        NATIVECALL(fclose(stream));
        free(stream_buffer);
    }
    if (fd != 0) {
        NATIVECALL(close(fd));
    }
}

FILE* SaveFile::open(const char *modes) {
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
            stream = open_memstream(&stream_buffer, &stream_size);
            fd = fileno(stream);
            return stream;
        }
    }

    if (stream == nullptr) {

        /* Open a new memory stream using pointers to these entries */
        stream = open_memstream(&stream_buffer, &stream_size);
        fd = fileno(stream);

        if (strstr(modes, "w") == nullptr) {
            /* Append the content of the file to the stream if the file exists */
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
                        fwrite(tmp_buf, 1, s, stream);
                    } while(s != 0);

                    fclose(f);
                }
            }
        }

        return stream;
    }

    /*
     * If we already opened the savefile:
     *   if opening in read, we seek at the beginning of the file
     *   if opening in append mode, we seek at the end of the file
     *   if opening in write mode, we create a new memstream (we cannot
     *   truncate the current one).
     */
    if (strstr(modes, "w") != nullptr) {
        fclose(stream);
        free(stream_buffer);

        stream = open_memstream(&stream_buffer, &stream_size);
        fd = fileno(stream);
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
        if (flags & O_RDONLY) {
            /* File was removed and opened in read-only mode */
            errno = ENOENT;
            closed = true;
            return -1;
        }
        else {
            /* File was removed and opened in write mode */
            open("wb");
        }
    }
    else {
        /* Create an anonymous file and store its file descriptor using memfd_create syscall. */
        open("wb");

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
        NATIVECALL(fclose(stream));
        stream = nullptr;
        free(stream_buffer);
        stream_buffer = nullptr;
        stream_size = 0;
        fd = 0;
    }

    if (fd != 0) {
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
        NATIVECALL(fclose(stream));
        stream = nullptr;
        free(stream_buffer);
        stream_buffer = nullptr;
        stream_size = 0;
        fd = 0;
    }

    if (fd != 0) {
        NATIVECALL(close(fd));
        fd = 0;
    }

    return 0;
}

}
