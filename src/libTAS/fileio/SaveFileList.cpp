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

#include "SaveFileList.h"

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "SaveFile.h"
#include "../global.h" // shared_config
#include "../GlobalState.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <memory>
#include <cstring>

namespace libtas {

namespace SaveFileList {

std::vector<std::unique_ptr<SaveFile>> savefiles;

/* Check if the file open permission allows for write operation */
bool isSaveFile(const char *file, const char *modes)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return true;
        }
    }

    if (!(strstr(modes, "w") || strstr(modes, "a") || strstr(modes, "+")))
        return false;

    return isSaveFile(file);
}

bool isSaveFile(const char *file, int oflag)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return true;
        }
    }

    if ((oflag & 0x3) == O_RDONLY)
        return false;

    /*
     * This is a sort of hack to prevent considering new shared
     * memory files as a savefile, which are opened using O_CLOEXEC
     */
    if (oflag & O_CLOEXEC)
        return false;

    return isSaveFile(file);
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

FILE *openSaveFile(const char *file, const char *modes)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return savefile->open(modes);
        }
    }

    savefiles.emplace_back(new SaveFile(file));
    return savefiles.back()->open(modes);
}

int openSaveFile(const char *file, int oflag)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return savefile->open(oflag);
        }
    }

    savefiles.emplace_back(new SaveFile(file));
    return savefiles.back()->open(oflag);
}

int closeSaveFile(int fd)
{
    for (const auto& savefile : savefiles) {
        if (savefile->fd == fd) {
            /* For now, do nothing */
            return 0;
        }
    }

    return 1;
}

int closeSaveFile(FILE *stream)
{
    for (const auto& savefile : savefiles) {
        if (savefile->stream == stream) {
            /* For now, do nothing */
            return 0;
        }
    }

    return 1;
}

int removeSaveFile(const char *file)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return savefile->remove();
        }
    }

    return 1;
}

int renameSaveFile(const char *oldfile, const char *newfile)
{
    std::string oldfilestr(oldfile);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(oldfilestr) == 0) {
            savefile->filename = std::string(newfile);
            return 0;
        }
    }

    return 1;
}

int getSaveFileFd(const char *file)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return savefile->fd;
        }
    }

    return 0;
}

bool isSaveFileRemoved(const char *file)
{
    std::string filestr(file);
    for (const auto& savefile : savefiles) {
        if (savefile->filename.compare(filestr) == 0) {
            return savefile->removed;
        }
    }

    return true;
}


}

}

#endif
