/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../GlobalState.h"
#include <csignal>

#define ONE_MB 1024 * 1024

namespace libtas {

/* We keep track of the alternate stack that the game is using. Indeed some
 * games do create an alternate stack such as FNA games (Towerfall, FEZ, etc.)
 * We switch to our own alternate stack just before checkpoint/restore.
 */
static stack_t oss;

void AltStack::saveStack()
{
    int ret;
    NATIVECALL(ret = sigaltstack(nullptr, &oss));
    MYASSERT(ret == 0)
}

void AltStack::prepareStack()
{
    /* Setup an alternate signal stack using our reserved memory */
    stack_t ss;
    ss.ss_sp = ReservedMemory::getAddr(ReservedMemory::STACK_ADDR);
    ss.ss_size = ReservedMemory::STACK_SIZE;
    ss.ss_flags = 0;

    int ret;
    NATIVECALL(ret = sigaltstack(&ss, nullptr));
    MYASSERT(ret == 0)
}

void AltStack::restoreStack()
{
    /* Restore the game altstack if any */
    int ret;
    NATIVECALL(ret = sigaltstack(&oss, nullptr));
    MYASSERT(ret == 0)
}

}
