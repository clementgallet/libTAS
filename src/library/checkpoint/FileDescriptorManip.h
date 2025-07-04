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

#ifndef LIBTAS_FILEDESCRIPTORMANIP_H
#define LIBTAS_FILEDESCRIPTORMANIP_H

namespace libtas {
namespace FileDescriptorManip
{
    /* Open as many fds as necessary until we get past `fd`, and
     * returns which last fd was opened. */
    int reserveUntil(int fd);

    /* Close all opened fds */
    void closeAll();

    /* Open as many fds as necessay, so that the next opened fd will be `fd`.
     * Returns which fd will be opened next. */
    int enforceNext(int fd);
    
    /* In the case of savestating, use one specific value for reserving fds */
    int reserveState();
    int reserveUntilState();
}
}

#endif
