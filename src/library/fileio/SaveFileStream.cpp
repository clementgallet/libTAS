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

#include "SaveFileStream.h"

#include "GlobalState.h"
#include "logging.h"

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

namespace libtas {

struct savefile_cookie_t
{
    char *addr;
    size_t size;
    off64_t pos;
};

static ssize_t savefile_read (void *cookie, char *b, size_t s)
{
    savefile_cookie_t *c = static_cast<savefile_cookie_t*>(cookie);
    
    if (c->pos + s > c->size) {
        LOG(LL_WARN, LCF_FILEIO, "Custom savefile stream read past the end, unsupported");
        s = c->size - c->pos;
    }
    
    memcpy (b, c->addr + c->pos, s);

    c->pos += s;
    return s;
}


static ssize_t savefile_write (void *cookie, const char *b, size_t s)
{
    savefile_cookie_t *c = static_cast<savefile_cookie_t*>(cookie);
    LOG(LL_DEBUG, LCF_FILEIO, "savefile_write called with addr %p and size %zu", c->addr, s);

    if (c->pos + s > c->size) {
        if (c->pos >= c->size) {
            LOG(LL_WARN, LCF_FILEIO, "Custom savefile stream written past the end, unsupported");
            errno = ENOSPC;
            return 0;
        }
        s = c->size - c->pos;
    }

    memcpy (c->addr + c->pos, b, s);
    c->pos += s;

    return s;
}

static int savefile_seek (void *cookie, off64_t *p, int w)
{
    savefile_cookie_t *c = static_cast<savefile_cookie_t*>(cookie);
    off_t np;

    switch (w) {
        case SEEK_SET:
            np = *p;
            break;
        case SEEK_CUR:
            np = c->pos + *p;
            break;
        case SEEK_END:
            np = c->size - *p;
            break;
        default:
            return -1;
    }

    if (np < 0 || (size_t) np > c->size)
        return -1;

    *p = c->pos = np;

    return 0;
}

static int savefile_close (void *cookie)
{
    savefile_cookie_t *c = static_cast<savefile_cookie_t*>(cookie);
    if (c->addr)
        munmap(c->addr, c->size);
    delete c;

    return 0;
}

FILE *SaveFileStream::open (const char *path, const char *mode)
{
    GlobalNative gn;
    
    /* Check if file exists and get size */
    struct stat filestat;
    int rv = stat(path, &filestat);

    if (rv < 0)
        return nullptr;
        
    if (! S_ISREG(filestat.st_mode) || (filestat.st_size == 0))
        return nullptr;

    /* Open and map the file with private mapping */
    int fd = ::open(path, O_RDONLY);
    if (fd < 0)
        return nullptr;

    void* addr = mmap(NULL, filestat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    
    close(fd);

    if (addr == MAP_FAILED)
        return nullptr;

    /* Prepare the custom stream to operate on this memory mapping */
    savefile_cookie_t *c = new savefile_cookie_t;

    c->addr = static_cast<char*>(addr);
    c->size = filestat.st_size;
    if (mode[0] == 'a')
        c->pos = c->size;
    else
        c->pos = 0;

    cookie_io_functions_t iof;
    iof.read = savefile_read;
    iof.write = savefile_write;
    iof.seek = savefile_seek;
    iof.close = savefile_close;

    FILE *result = fopencookie (c, mode, iof);
    if (result == nullptr) {
        munmap(c->addr, c->size);
        delete c;
    }

    return result;
}

}
