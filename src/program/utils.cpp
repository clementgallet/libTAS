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

#include "utils.h"
#include <sys/stat.h>
#include <cerrno> // errno
#include <cstring> // strerror
#include <iostream>
#include <unistd.h> // unlink

int create_dir(std::string& path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1) {
        if (errno == ENOENT) {
            /* The directory does not exist, try to create it */
            int dir_err = mkdir(path.c_str(), S_IRWXU);
            if (dir_err == -1) {
                std::cerr << "Error creating directory " << path << ": " << strerror(errno) << std::endl;
                return -1;
            }
        }
        else {
            std::cerr << "Error accessing directory " << path << ": " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        std::cerr << "Something has the same name as our prefs directory " << path << std::endl;
        return -1;
    }
    return 0;
}

void remove_savestates(Context* context)
{
    std::string savestateprefix = context->config.savestatedir + '/';
    savestateprefix += context->gamename;
    for (int i=0; i<=10; i++) {
        std::string savestatepmpath = savestateprefix + ".state" + std::to_string(i) + ".pm";
        unlink(savestatepmpath.c_str());
        std::string savestatepspath = savestateprefix + ".state" + std::to_string(i) + ".p";
        unlink(savestatepspath.c_str());
    }
}
