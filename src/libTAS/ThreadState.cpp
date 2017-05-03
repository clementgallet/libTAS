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

thread_local int ThreadState::native = 0;
thread_local int ThreadState::owncode = 0;
thread_local int ThreadState::nolog = 0;

void ThreadState::setNative(bool state)
{
    if (state)
        native++;
    else
        native--;
    setOwnCode(state);
}

bool ThreadState::isNative()
{
    return (native > 0);
}

void ThreadState::setOwnCode(bool state)
{
    if (state)
        owncode++;
    else
        owncode--;
    setNoLog(state);
}

bool ThreadState::isOwnCode()
{
    return (owncode > 0);
}

void ThreadState::setNoLog(bool state)
{
    if (state)
        nolog++;
    else
        nolog--;
}

bool ThreadState::isNoLog()
{
    return (nolog > 0);
}

ThreadNative::ThreadNative()
{
    ThreadState::setNative(true);
}

ThreadNative::~ThreadNative()
{
    ThreadState::setNative(false);
}

ThreadOwnCode::ThreadOwnCode()
{
    ThreadState::setOwnCode(true);
}

ThreadOwnCode::~ThreadOwnCode()
{
    ThreadState::setOwnCode(false);
}

ThreadNoLog::ThreadNoLog()
{
    ThreadState::setNoLog(true);
}

ThreadNoLog::~ThreadNoLog()
{
    ThreadState::setNoLog(false);
}
