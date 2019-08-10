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

#ifndef LIBTAS_FILEHANDLE_H_INCLUDED
#define LIBTAS_FILEHANDLE_H_INCLUDED

#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace libtas {

struct FileHandle {
    FileHandle(const char *file, int fd)
        : fds{fd, -1}, stream(nullptr), fileNameOrPipeContents(::strdup(file)), fileOffset(-1),
          tracked(false), closed(false) {}
    FileHandle(const char *file, FILE* f)
        : fds{fileno(f), -1}, stream(f), fileNameOrPipeContents(::strdup(file)), fileOffset(-1),
          tracked(false), closed(false) {}
    FileHandle(int fds[2])
        : fds{fds[0], fds[1]}, stream(nullptr), fileNameOrPipeContents(nullptr), pipeSize(-1),
          tracked(false), closed(false) {}
    ~FileHandle() { std::free(fileNameOrPipeContents); }
    bool isPipe() const { return fds[1] != -1; }
    const char *fileName() const { return isPipe() ? "pipe" : fileNameOrPipeContents; }
    off_t offset() const { return isPipe() ? fileOffset : pipeSize; }

    /* File descriptor(s) */
    int fds[2];

    /* File stream if one */
    FILE* stream;

    /* Path of the file */
    /* or Saved contents of the pipe */
    char *fileNameOrPipeContents;

    union {
        /* Saved offset in the file */
        off_t fileOffset;

        /* Saved size of the pipe */
        int pipeSize;
    };

    /* Are we tracking this file? */
    bool tracked;

    /* Was the file closed? */
    bool closed;
};

}

#endif
