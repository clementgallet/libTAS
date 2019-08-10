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
#include "FileHandleList.h"
#include "../GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(fopen)
DEFINE_ORIG_POINTER(fopen64)
DEFINE_ORIG_POINTER(fclose)
// DEFINE_ORIG_POINTER(fileno)

FILE *fopen (const char *filename, const char *modes)
{
    LINK_NAMESPACE_GLOBAL(fopen);

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

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(filename, modes);
    }

    FILE* f = orig::fopen(filename, modes);

    /* Store the file descriptor */
    if (f) {
        FileHandleList::openFile(filename, f);
    }

    return f;
}

FILE *fopen64 (const char *filename, const char *modes)
{
    LINK_NAMESPACE_GLOBAL(fopen64);

    if (GlobalState::isNative())
        return orig::fopen64(filename, modes);

    if (filename)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and mode %s", __func__, filename, modes);
    else
        debuglogstdio(LCF_FILEIO, "%s call with null filename", __func__);

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        return SaveFileList::openSaveFile(filename, modes);
    }

    FILE* f = orig::fopen64(filename, modes);

    /* Store the file descriptor */
    if (f) {
        FileHandleList::openFile(filename, f);
    }

    return f;
}

int fclose (FILE *stream)
{
    LINK_NAMESPACE_GLOBAL(fclose);

    if (GlobalState::isNative())
        return orig::fclose(stream);

    DEBUGLOGCALL(LCF_FILEIO);

    int ret = SaveFileList::closeSaveFile(stream);
    if (ret != 1)
        return ret;

    /* Check if we must actually close the file */
    bool doClose = FileHandleList::closeFile(fileno(stream));

    if (doClose) {
        return orig::fclose(stream);
    }

    return 0;
}

// int fileno (FILE *stream) throw()
// {
//     LINK_NAMESPACE_GLOBAL(fileno);
//
//     if (GlobalState::isNative())
//         return orig::fileno(stream);
//
//     DEBUGLOGCALL(LCF_FILEIO);
//
//     return orig::fileno(stream);
// }

}
