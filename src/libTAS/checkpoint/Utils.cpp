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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "Utils.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace libtas {

// Fails or does entire write (returns count)
ssize_t Utils::writeAll(int fd, const void *buf, size_t count)
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
ssize_t Utils::readAll(int fd, void *buf, size_t count)
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

/* This function detects if the given pages are zero pages or not. There is
 * scope of improving this function using some optimizations.
 *
 * TODO: One can use /proc/self/pagemap to detect if the page is backed by a
 * shared zero page.
 */
bool Utils::areZeroPages(void *addr, size_t numPages)
{
    static const size_t page_size = sysconf(_SC_PAGESIZE);
    long long *buf = (long long *)addr;
    size_t end = numPages * page_size / sizeof(*buf);
    long long res = 0;

    for (size_t i = 0; i + 7 < end; i += 8) {
        res = buf[i + 0] | buf[i + 1] | buf[i + 2] | buf[i + 3] |
        buf[i + 4] | buf[i + 5] | buf[i + 6] | buf[i + 7];
        if (res != 0) {
            break;
        }
    }
    return res == 0;
}

}
