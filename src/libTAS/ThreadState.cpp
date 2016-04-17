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

#include "ThreadState.h"

thread_local ThreadState threadState;

void ThreadState::setNative(bool state)
{
    static int callcount = 0;
    if (state) {
        if (!callcount++)
            stateMask |= NATIVE;
    }
    else {
        if (!--callcount)
            stateMask &= ~NATIVE;
    }
    setOwnCode(state);
}

bool ThreadState::isNative()
{
    return (stateMask & NATIVE);
}

void ThreadState::setOwnCode(bool state)
{
    static int callcount = 0;
    if (state) {
        if (!callcount++)
            stateMask |= OWNCODE;
    }
    else {
        if (!--callcount)
            stateMask &= ~OWNCODE;
    }
    setNoLog(state);
}

bool ThreadState::isOwnCode()
{
    return (stateMask & OWNCODE);
}

void ThreadState::setNoLog(bool state)
{
    static int callcount = 0;
    if (state) {
        if (!callcount++)
            stateMask |= NOLOG;
    }
    else {
        if (!--callcount)
            stateMask &= ~NOLOG;
    }
}

bool ThreadState::isNoLog()
{
    return (stateMask & NOLOG);
}


