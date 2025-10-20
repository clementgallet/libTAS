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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "FileDescriptorManip.h"

#include <unistd.h> // dup

namespace libtas {

/* Array of fds used to obtain the correct fd values */
static int tmp_fds[256] = {};
static int tmp_fds_index = -1;

int FileDescriptorManip::reserveUntil(int fd)
{
    tmp_fds_index++;
    tmp_fds[tmp_fds_index] = dup(0);
    while ((tmp_fds[tmp_fds_index] != -1) && (tmp_fds[tmp_fds_index] < fd)) {
        tmp_fds_index++;
        tmp_fds[tmp_fds_index] = dup(0);
    }
    
    return tmp_fds[tmp_fds_index];
}

void FileDescriptorManip::closeAll()
{
    for (int i = tmp_fds_index; i >= 0; i--) {
        if (tmp_fds[i] > 0) {
            close(tmp_fds[i]);
            tmp_fds[i] = 0;
        }
    }
    tmp_fds_index = -1;
}

int FileDescriptorManip::enforceNext(int fd)
{
    int last_fd = reserveUntil(fd);
    if (last_fd != -1) {
        close(last_fd);
        tmp_fds[tmp_fds_index] = 0;
        tmp_fds_index--;
    }
    
    return last_fd;
}

int FileDescriptorManip::reserveState()
{
    return 240;
}

int FileDescriptorManip::reserveUntilState()
{
    return reserveUntil(reserveState());
}

}
