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
#include <zlib.h>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_ACCMODE, O_CREAT

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

/* We store the gz struct so it can be used across all wrapper functions.
 * We need to do this because standard open/read/write/close functions use a
 * file descriptor (int), but gzopen/gzread/gzwrite/gzclose functions use a
 * pointer to a struct, which is larger than an int on some archs.
 */
gzFile gzf;

int gzopen_wrapper(const char *pathname, int oflags, int mode)
{
	const char *gzoflags;

	switch (oflags & O_ACCMODE) {
	case O_WRONLY:
		gzoflags = "wb1";
		break;
	case O_RDONLY:
		gzoflags = "rb";
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	int fd = open(pathname, oflags, mode);
	if (fd == -1)
		return -1;

	if ((oflags & O_CREAT) && fchmod(fd, mode)) {
		close(fd);
		return -1;
	}

	gzf = gzdopen(fd, gzoflags);
	if (!gzf) {
		errno = ENOMEM;
		return -1;
	}

	return fd;
}

ssize_t gzread_wrapper(int, void *buf, size_t count)
{
    return gzread(gzf, buf, count);
}

ssize_t gzwrite_wrapper(int, const void *buf, size_t count)
{
    return gzwrite(gzf, buf, count);
}

int gzclose_wrapper(int)
{
    return gzclose(gzf);
}

void remove_savestates(Context* context)
{
    std::string savestateprefix = context->config.savestatedir + '/';
    savestateprefix += context->gamename;
    for (int i=1; i<=9; i++) {
        std::string savestatepath = savestateprefix + ".state" + std::to_string(i);
        unlink(savestatepath.c_str());
        std::string moviepath = savestateprefix + ".movie" + std::to_string(i) + ".ltm";
        unlink(moviepath.c_str());
    }
}
