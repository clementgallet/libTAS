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

#include "CustomSignals.h"
#include "ThreadManager.h"
#include "Checkpoint.h"
#include "../logging.h"
#include <csignal>

namespace libtas {

/* We keep track of the signal handlers for SIGUSR1 and SIGUSR2 that the game
 * might be using. We switch to our signals just before checkpoint/restore.
 */
static struct sigaction act_usr1, act_usr2;

void CustomSignals::registerHandlers()
{
    struct sigaction sigusr1;
    sigfillset(&sigusr1.sa_mask);
    sigusr1.sa_flags = SA_RESTART | SA_ONSTACK;
    sigusr1.sa_handler = ThreadManager::stopThisThread;
    {
        GlobalNative gn;
        MYASSERT(sigaction(SIGUSR1, &sigusr1, &act_usr1) == 0)
    }

    struct sigaction sigusr2;
    sigfillset(&sigusr2.sa_mask);
    sigusr2.sa_flags = SA_ONSTACK;
    sigusr2.sa_handler = Checkpoint::handler;
    {
        GlobalNative gn;
        MYASSERT(sigaction(SIGUSR2, &sigusr2, &act_usr2) == 0)
    }
}

void CustomSignals::restoreHandlers()
{
    GlobalNative gn;
    MYASSERT(sigaction(SIGUSR1, &act_usr1, nullptr) == 0)
    MYASSERT(sigaction(SIGUSR2, &act_usr2, nullptr) == 0)
}

}
