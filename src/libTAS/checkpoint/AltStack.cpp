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

#include "AltStack.h"
#include "ReservedMemory.h"
#include "../logging.h"
#include <csignal>

#define ONE_MB 1024 * 1024

namespace libtas {

static stack_t oss;

void AltStack::prepareStack()
{
    /* Setup an alternate signal stack using our reserved memory */
    stack_t ss;
    ss.ss_sp = ReservedMemory::getAddr(ONE_MB);
    ss.ss_size = ReservedMemory::getSize() - ONE_MB;
    ss.ss_flags = 0;
    MYASSERT(sigaltstack(&ss, &oss) == 0)
}

void AltStack::restoreStack()
{
    /* Restore the game altstack if any */
    MYASSERT(sigaltstack(&oss, nullptr) == 0)
}

}
