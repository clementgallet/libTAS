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

#ifndef LIBTAS_SAVEFILE_H_INCLUDED
#define LIBTAS_SAVEFILE_H_INCLUDED

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include <string>
#include <cstdio> // FILE

namespace libtas {

class SaveFile {

public:
    SaveFile(const char *file);
    ~SaveFile();

    std::string filename;

    FILE* stream;
    char* stream_buffer;
    size_t stream_size;

    int fd;

    bool removed = false;

    /* Open and return a FILE stream of the savefile */
    FILE* open(const char *modes);

    /* Open and return a file descriptor of the savefile */
    int open(int flags);

    /* Remove a savefile and return 0 for success and -1 for error (+ errno set) */
    int remove();

};

}

#endif
#endif
