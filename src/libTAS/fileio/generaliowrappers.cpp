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

#include "generaliowrappers.h"

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "../logging.h"
#include "../hook.h"
#include "stdiowrappers.h"
#include "posixiowrappers.h"
#include "../GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(rename)
DEFINE_ORIG_POINTER(remove)
DEFINE_ORIG_POINTER(unlink)

int rename (const char *oldf, const char *newf) throw()
{
    LINK_NAMESPACE(rename, nullptr);

    if (GlobalState::isNative())
        return orig::rename(oldf, newf);

    debuglogstdio(LCF_FILEIO, "%s call with old %s and new %s", __func__, oldf?oldf:"<NULL>", newf?newf:"<NULL>");

    /* Check if file is a savefile */
    if (rename_stdio(oldf, newf)) {
        return 0;
    }

    if (rename_posix(oldf, newf)) {
        return 0;
    }

    return orig::rename(oldf, newf);
}

/* Remove file FILENAME.  */
int remove (const char *filename) throw()
{
    LINK_NAMESPACE(remove, nullptr);

    if (GlobalState::isNative())
        return orig::remove(filename);

    debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, filename?filename:"<NULL>");

    /* Check if file is a savefile */
    if (remove_stdio(filename)) {
        return 0;
    }

    if (remove_posix(filename)) {
        return 0;
    }

    return orig::remove(filename);
}

/* Remove the link NAME.  */
int unlink (const char *name) throw()
{
    LINK_NAMESPACE(unlink, nullptr);

    if (GlobalState::isNative())
        return orig::unlink(name);

    debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, name?name:"<NULL>");

    /* Check if file is a savefile */
    if (remove_stdio(name)) {
        return 0;
    }

    if (remove_posix(name)) {
        return 0;
    }

    return orig::unlink(name);
}

}

#endif
