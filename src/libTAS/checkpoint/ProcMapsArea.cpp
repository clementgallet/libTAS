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

#include "ProcMapsArea.h"
#include "../logging.h"
#include <sys/mman.h> // PROT_READ, PROT_WRITE, etc.

namespace libtas {

void Area::print(const char* prefix) const
{
    debuglogstdio(LCF_CHECKPOINT, "%s Region %c%c%c%c %p (%s) with size %d",
    prefix,
    (prot&PROT_READ)?'r':'-', (prot&PROT_WRITE)?'w':'-',
    (prot&PROT_EXEC)?'x':'-', (flags&MAP_SHARED)?'s':'p',
    addr, name, size);
}

}
