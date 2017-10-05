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

#include "control.h"
#include "../../logging.h"
#include "../../GlobalState.h"
#include "../../hook.h"
#include "../../threadwrappers.h" // isMainThread()

namespace libtas {

int snd_device_name_hint(int card, const char *iface, void ***hints)
{
    debuglog(LCF_SOUND, __func__, " call with card ", card, " and iface ", iface);
    return 0;
}

int snd_device_name_free_hint(void **hints)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

char *snd_device_name_get_hint(const void *hint, const char *id)
{
    DEBUGLOGCALL(LCF_SOUND);
    static char device_name[] = "libTAS dummy device name";
    static char device_desc[] = "libTAS dummy device description";
    static char device_io[] = "Output";
    static char device_other[] = "";

    if (strcmp(id, "NAME") == 0) {
        return device_name;
    }

    if (strcmp(id, "DESC") == 0) {
        return device_desc;
    }

    if (strcmp(id, "IOID") == 0) {
        return device_io;
    }

    return device_other;
}

}
