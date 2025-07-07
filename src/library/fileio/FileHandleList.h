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

#ifndef LIBTAS_FILEHANDLELIST_H_INCLUDED
#define LIBTAS_FILEHANDLELIST_H_INCLUDED

#include <utility>
#include <cstdio>

namespace libtas {

class FileHandle;

namespace FileHandleList {

/* Open and register an unnamed pipe */
std::pair<int, int> createPipe(int flags = 0);

/* Return the file descriptor from a filename */
int fdFromFile(const char* file);

/* Return a registered file handle from a file descriptor */
const FileHandle& fileHandleFromFd(int fd);

/* Scan list of file descriptors using /proc/self/fd, and add all file descriptors */
void trackAllFiles();

/* Save offset and size of file handle */
void trackFile(FileHandle &fh);

/* Recover the offset of all tracked files */
void recoverAllFiles();

}

}

#endif
