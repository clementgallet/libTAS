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
 */

#ifndef LINTAS_STATESECTION_H_INCLUDED
#define LINTAS_STATESECTION_H_INCLUDED

#include <sys/uio.h>
#include <string>
#include <vector>

/* Store a section of the game memory */
class StateSection {
    public:
        /* All information gather from a single line of /proc/pid/maps */
        uintptr_t addr;
        uintptr_t endaddr;
        intptr_t size;
        bool readflag;
        bool writeflag;
        bool execflag;
        bool sharedflag;
        int offset;
        std::string device;
        int inode;
        std::string filename;

        /* The actual memory inside this section */
        std::vector<uint8_t> mem;

        /* Parse a single line from the /proc/pid/maps file. */
        void readMap(std::string& line);

        void toIovec(struct iovec& local, struct iovec& remote);

};

#endif
