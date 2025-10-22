/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
    
    enum {
        FILE_REGULAR,
        FILE_PIPE,
        FILE_SOCKET,
        FILE_MEMFD,
        FILE_DEVICE,
        FILE_SPECIAL,
    };

    FileHandle() : fds{-1, -1} {}
    FileHandle(const char *file, int fd, int t)
        : type(t), fds{fd, -1}, fileName(::strdup(file)), fileOffset(-1),
          size(-1) {}
    FileHandle(const char *file, int fds[2])
        : type(FileHandle::FILE_PIPE), fds{fds[0], fds[1]}, fileName(::strdup(file)), fileOffset(-1),
          size(-1) {}
    ~FileHandle() { std::free(fileName); std::free(pipeContents); }
    bool needsTracking() const
    {
        return (type == FileHandle::FILE_REGULAR) || (type == FileHandle::FILE_MEMFD);
    }
    static const char* typeStr(int type) {
        switch (type) {
            case FILE_REGULAR:
                return "Regular";
            case FILE_PIPE:
                return "Pipe";
            case FILE_SOCKET:
                return "Socket";
            case FILE_MEMFD:
                return "Savefile";
            case FILE_DEVICE:
                return "Device";
            case FILE_SPECIAL:
                return "Special";
        }
        return "";
    }

    /* File type */
    int type;

    /* File descriptor(s) */
    int fds[2];

    /* Path of the file */
    char *fileName;

    /* Saved offset in the file */
    off_t fileOffset;

    /* Saved size of the file or pipe */
    off_t size;

    /* Saved contents of the pipe */
    char *pipeContents;
};

}

#endif
