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
 */

#include "GameHacks.h"
#include "logging.h"

namespace libtas {

static bool coreclr = false;
static pid_t finalizer_pid = 0;

void GameHacks::setCoreclr()
{
    debuglogstdio(LCF_HOOK, "   detected coreclr");
    coreclr = true;
}

bool GameHacks::hasCoreclr()
{
    return coreclr;
}

void GameHacks::setFinalizerThread(pid_t pid)
{
    debuglogstdio(LCF_HOOK, "   set finalizer to %d", pid);
    finalizer_pid = pid;
}

pid_t GameHacks::getFinalizerThread()
{
    return finalizer_pid;
}

}
