/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "Utils.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace Utils
{
    // Fails or does entire write (returns count)
    ssize_t writeAll(int fd, const void *buf, size_t count)
    {
        const char *ptr = (const char *)buf;
        size_t num_written = 0;

        do {
            ssize_t rc = write(fd, ptr + num_written, count - num_written);
            if (rc == -1) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                } else {
                    return rc;
                }
            } else if (rc == 0) {
                break;
            } else { // else rc > 0
                num_written += rc;
            }
        } while (num_written < count);
        MYASSERT(num_written == count);
        return num_written;
    }

    // Fails, succeeds, or partial read due to EOF (returns num read)
    // return value:
    // -1: unrecoverable error
    // <n>: number of bytes read
    ssize_t readAll(int fd, void *buf, size_t count)
    {
        ssize_t rc;
        char *ptr = (char *)buf;
        size_t num_read = 0;

        for (num_read = 0; num_read < count;) {
            rc = read(fd, ptr + num_read, count - num_read);
            if (rc == -1) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                } else {
                    return -1;
                }
            } else if (rc == 0) {
                break;
            } else { // else rc > 0
                num_read += rc;
            }
        }
        return num_read;
    }
}
