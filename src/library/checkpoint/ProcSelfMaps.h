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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#ifndef LIBTAS_PROCSELFMAPS_H
#define LIBTAS_PROCSELFMAPS_H

#include "MemArea.h"

namespace libtas {
class ProcSelfMaps
{
    public:
        /* Read the /proc/self/maps file into reserved memory */
        ProcSelfMaps();
        ~ProcSelfMaps();

        /* Parse the next memory section into the area */
        bool getNextArea(Area *area);

        /* Reset all internal variables */
        void reset();

    private:
        uintptr_t readDec();
        uintptr_t readHex();

        int tmp_fd;
        off_t off;
        
        char line[1024];
        int line_idx;
};
}

#endif
