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

#include "stdiowrappers.h"

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
#include "../GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(fopen)
DEFINE_ORIG_POINTER(fopen64)
DEFINE_ORIG_POINTER(fclose)

FILE *fopen (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen, nullptr);

    if (GlobalState::isNative())
        return orig::fopen(filename, modes);

    /* iostream functions are banned inside this function, I'm not sure why.
     * This is the case for every open function.
     * Generating debug message using stdio.
     */
    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = SaveFileList::openSaveFile(filename, modes);
    }
    else
        f = orig::fopen(filename, modes);

    return f;
}

FILE *fopen64 (const char *filename, const char *modes)
{
    LINK_NAMESPACE(fopen64, nullptr);

    if (GlobalState::isNative())
        return orig::fopen64(filename, modes);

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    FILE* f;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = SaveFileList::openSaveFile(filename, modes);
    }
    else
        f = orig::fopen64(filename, modes);

    return f;
}

int fclose (FILE *stream)
{
    LINK_NAMESPACE(fclose, nullptr);

    if (GlobalState::isNative())
        return orig::fclose(stream);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    int ret = SaveFileList::closeSaveFile(stream);
    if (ret != 1)
        return ret;

    int rv = orig::fclose(stream);

    return rv;
}

}
