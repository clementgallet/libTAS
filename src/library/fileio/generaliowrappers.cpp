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

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
#include "../GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(rename)
DEFINE_ORIG_POINTER(remove)
DEFINE_ORIG_POINTER(unlink)

int rename (const char *oldf, const char *newf) throw()
{
    LINK_NAMESPACE_GLOBAL(rename);

    if (GlobalState::isNative())
        return orig::rename(oldf, newf);

    debuglogstdio(LCF_FILEIO, "%s call with old %s and new %s", __func__, oldf?oldf:"<NULL>", newf?newf:"<NULL>");

    /* Check if file is a savefile */
    int ret = SaveFileList::renameSaveFile(oldf, newf);
    if (ret != 1) {
        return ret;
    }

    return orig::rename(oldf, newf);
}

/* Remove file FILENAME.  */
int remove (const char *filename) throw()
{
    LINK_NAMESPACE_GLOBAL(remove);

    if (GlobalState::isNative())
        return orig::remove(filename);

    debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, filename?filename:"<NULL>");

    /* Check if file is a savefile */
    int ret = SaveFileList::removeSaveFile(filename);
    if (ret != 1) {
        return ret;
    }

    return orig::remove(filename);
}

/* Remove the link NAME.  */
int unlink (const char *name) throw()
{
    LINK_NAMESPACE_GLOBAL(unlink);

    if (GlobalState::isNative())
        return orig::unlink(name);

    debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, name?name:"<NULL>");

    /* Check if file is a savefile */
    int ret = SaveFileList::removeSaveFile(name);
    if (ret != 1) {
        return ret;
    }

    return orig::unlink(name);
}

}
