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

#ifndef LIBTAS_SAVESTATE_H
#define LIBTAS_SAVESTATE_H

#include "ProcMapsArea.h"

namespace libtas {
class SaveState
{
    public:
        SaveState(char* pagemappath, char* pagespath);
        ~SaveState();

        char getPageFlag(char* addr);

        explicit operator bool() const {
            return (pmfd != -1);
        }

        char page[4096];

    private:
        int pmfd, pfd;

        Area area;
        char* current_addr;


};
}

#endif
