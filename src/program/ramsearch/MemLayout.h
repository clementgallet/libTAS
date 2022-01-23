/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_MEMLAYOUT_H_INCLUDED
#define LIBTAS_MEMLAYOUT_H_INCLUDED

#include <string>
#include <fstream>
#include "MemSection.h"

/* Handle the layout of game memory */
class MemLayout {
    public:

        MemLayout(pid_t pid);
        ~MemLayout();
        
        /* Compute the total size of memory regions, within the selected `types`
         * and following the selected `flags` */
        uint64_t totalSize(int types, int flags);
        
        bool nextSection(int types, int flags, MemSection &section);

    private:
        void readLayout();

        pid_t pid;
        std::ifstream mapsfile;
};

#endif
