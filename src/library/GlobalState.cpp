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
 */

#include "GlobalState.h"
#include "global.h"

namespace libtas {

thread_local int GlobalState::native = 0;
thread_local int GlobalState::owncode = 0;
thread_local int GlobalState::nolog = 0;

#if defined(__APPLE__) && defined(__MACH__)
static int native_init = 0;
static int owncode_init = 0;
static int nolog_init = 0;
#endif

void GlobalState::setNative(bool state)
{
    /* On MacOS, thread-local storage cannot be access while the library is being loaded. */
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited) {
        if (state)
            native_init++;
        else
            native_init--;
        return;
    }
#endif

    if (state)
        native++;
    else
        native--;
}

bool GlobalState::isNative()
{
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited)
        return (native_init > 0);
#endif

    return (native > 0);
}

void GlobalState::setOwnCode(bool state)
{
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited) {
        if (state)
            owncode_init++;
        else
            owncode_init--;
        return;
    }
#endif

    if (state)
        owncode++;
    else
        owncode--;
}

bool GlobalState::isOwnCode()
{
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited)
        return (owncode_init > 0);
#endif

    return (owncode > 0);
}

void GlobalState::setNoLog(bool state)
{
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited) {
        if (state)
            nolog_init++;
        else
            nolog_init--;
        return;
    }
#endif

    if (state)
        nolog++;
    else
        nolog--;
}

bool GlobalState::isNoLog()
{
#if defined(__APPLE__) && defined(__MACH__)
    if (!Global::is_inited)
        return (nolog_init > 0);
#endif

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
