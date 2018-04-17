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

#include "detectsavefiles.h"

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "../global.h" // shared_config
#include "../GlobalState.h"
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

namespace libtas {

/* Check if the file open permission allows for write operation */
bool isWriteable(const char *modes)
{
    if (strstr(modes, "w") || strstr(modes, "a") || strstr(modes, "+"))
        return true;
    return false;
}

bool isWriteable(int oflag)
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

/* Detect save files (excluding the writeable flag), basically if the file is regular */
bool isSaveFile(const char *file)
{
    if (!shared_config.prevent_savefiles)
        return false;

    if (!file)
        return false;

    /* Check if file is a dev file */
    GlobalNative gn;
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

}

#endif
