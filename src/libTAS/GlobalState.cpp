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
 */

#include "GlobalState.h"

namespace libtas {

thread_local int GlobalState::native = 0;
thread_local int GlobalState::owncode = 0;
thread_local int GlobalState::nolog = 0;

void GlobalState::setNative(bool state)
{
    if (state)
        native++;
    else
        native--;
}

bool GlobalState::isNative()
{
    return (native > 0);
}

void GlobalState::setOwnCode(bool state)
{
    if (state)
        owncode++;
    else
        owncode--;
}

bool GlobalState::isOwnCode()
{
    return (owncode > 0);
}

void GlobalState::setNoLog(bool state)
{
    if (state)
        nolog++;
    else
        nolog--;
}

bool GlobalState::isNoLog()
{
    return (nolog > 0);
}

GlobalNative::GlobalNative()
{
    GlobalState::setNative(true);
}

GlobalNative::~GlobalNative()
{
    GlobalState::setNative(false);
}

GlobalOwnCode::GlobalOwnCode()
{
    GlobalState::setOwnCode(true);
}

GlobalOwnCode::~GlobalOwnCode()
{
    GlobalState::setOwnCode(false);
}

GlobalNoLog::GlobalNoLog()
{
    GlobalState::setNoLog(true);
}

GlobalNoLog::~GlobalNoLog()
{
    GlobalState::setNoLog(false);
}

}
