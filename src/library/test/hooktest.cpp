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

#include "hooktest.h"
#include "../logging.h"
#include "../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(libtasTestFunc1);
DEFINE_ORIG_POINTER(libtasTestFunc2);
DEFINE_ORIG_POINTER(libtasTestFunc3);

/* Override */ int libtasTestFunc1()
{
    DEBUGLOGCALL(LCF_HOOK);
    LINK_NAMESPACE_GLOBAL(libtasTestFunc1);
    if (!orig::libtasTestFunc1) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Couldn't get original function");
    }
    else if (orig::libtasTestFunc1() != 1) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Original function gives wrong result");
    }
    else {
        debuglog(LCF_HOOK, "   Correctly linked to original function");
    }
    return 2;
}

/* Override */ int libtasTestFunc2()
{
    DEBUGLOGCALL(LCF_HOOK);
    LINK_NAMESPACE(libtasTestFunc2, "hooklib2");
    if (!orig::libtasTestFunc2) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Couldn't get original function");
    }
    else if (orig::libtasTestFunc2() != 1) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Original function gives wrong result");
    }
    else {
        debuglog(LCF_HOOK, "   Correctly linked to original function");
    }
    return 2;
}

/* Override */ int libtasTestFunc3()
{
    DEBUGLOGCALL(LCF_HOOK);
    LINK_NAMESPACE(libtasTestFunc3, "hooklib3");
    if (!orig::libtasTestFunc3) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Couldn't get original function");
    }
    else if (orig::libtasTestFunc3() != 1) {
        debuglog(LCF_HOOK | LCF_ERROR, "   Original function gives wrong result");
    }
    else {
        debuglog(LCF_HOOK, "   Correctly linked to original function");
    }
    return 2;
}

}
