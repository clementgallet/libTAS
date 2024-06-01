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

#include "generaliowrappers.h"
#include "SaveFileList.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"
#include "global.h"

namespace libtas {

int rename (const char *oldf, const char *newf) __THROW
{
    RETURN_IF_NATIVE(rename, (oldf, newf), nullptr);
    
    LOG(LL_TRACE, LCF_FILEIO, "%s call with old %s and new %s", __func__, oldf?oldf:"<NULL>", newf?newf:"<NULL>");

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        RETURN_NATIVE(rename, (oldf, newf), nullptr);

    /* Check if file is a savefile */
    int ret = SaveFileList::renameSaveFile(oldf, newf);
    if (ret != 1) {
        return ret;
    }

    RETURN_NATIVE(rename, (oldf, newf), nullptr);
}

/* Remove file FILENAME.  */
int remove (const char *filename) __THROW
{
    RETURN_IF_NATIVE(remove, (filename), nullptr);

    LOG(LL_TRACE, LCF_FILEIO, "%s call with file %s", __func__, filename?filename:"<NULL>");

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        RETURN_NATIVE(remove, (filename), nullptr);

    /* Check if file is a savefile */
    int ret = SaveFileList::removeSaveFile(filename);
    if (ret != 1) {
        return ret;
    }

    RETURN_NATIVE(remove, (filename), nullptr);
}

/* Remove the link NAME.  */
int unlink (const char *name) __THROW
{
    RETURN_IF_NATIVE(unlink, (name), nullptr);

    LOG(LL_TRACE, LCF_FILEIO, "%s call with file %s", __func__, name?name:"<NULL>");

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        RETURN_NATIVE(unlink, (name), nullptr);

    /* Check if file is a savefile */
    int ret = SaveFileList::removeSaveFile(name);
    if (ret != 1) {
        return ret;
    }

    RETURN_NATIVE(unlink, (name), nullptr);
}

}
