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

#include "stdiowrappers.h"

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
#include "FileHandleList.h"
#include "../GlobalState.h"
#include "URandom.h"

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

    FILE* f = nullptr;

    if ((strcmp(filename, "/dev/urandom") == 0) || (strcmp(filename, "/dev/random") == 0)) {
        return urandom_create_file();
    }

    else if (strcmp(filename, "/proc/uptime") == 0) {
        if (SaveFileList::getSaveFileFd(filename) == 0) {
            /* Create a file with memory storage (reusing the savefile code),
             * and fill it with values from the initial time, so that, for
             * games that use it as PRNG seed, tweaking the initial time will
             * change the seed value.
             */
            f = SaveFileList::openSaveFile(filename, "w");

            std::ostringstream datestr;
            datestr << shared_config.initial_time_sec << ".";
            datestr << std::setfill ('0') << std::setw (2);
            datestr << shared_config.initial_time_nsec / 10000000;

            std::string s = datestr.str();

            debuglogstdio(LCF_FILEIO, "Creating fake %s with %s", filename, s.c_str());
            fwrite(s.c_str(), sizeof(char), s.size(), f);
            fwrite(" ", sizeof(char), 1, f);
            fwrite(s.c_str(), sizeof(char), s.size(), f);
            fseek(f, 0, SEEK_SET);
        }
        else {
            f = SaveFileList::openSaveFile(filename, modes);
        }
    }

    else if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = SaveFileList::openSaveFile(filename, modes);
    }

    else {
        f = orig::fopen(filename, modes);
    }

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

    FILE* f = nullptr;

    if ((strcmp(filename, "/dev/urandom") == 0) || (strcmp(filename, "/dev/random") == 0)) {
        return urandom_create_file();
    }

    else if (strcmp(filename, "/proc/uptime") == 0) {
        if (SaveFileList::getSaveFileFd(filename) == 0) {
            /* Create a file with memory storage (reusing the savefile code),
             * and fill it with values from the initial time, so that, for
             * games that use it as PRNG seed, tweaking the initial time will
             * change the seed value.
             */
            f = SaveFileList::openSaveFile(filename, "w");

            std::ostringstream datestr;
            datestr << shared_config.initial_time_sec << ".";
            datestr << std::setfill ('0') << std::setw (2);
            datestr << shared_config.initial_time_nsec / 10000000;

            std::string s = datestr.str();

            debuglogstdio(LCF_FILEIO, "Creating fake %s with %s", filename, s.c_str());
            fwrite(s.c_str(), sizeof(char), s.size(), f);
            fwrite(" ", sizeof(char), 1, f);
            fwrite(s.c_str(), sizeof(char), s.size(), f);
            fseek(f, 0, SEEK_SET);
        }
        else {
            f = SaveFileList::openSaveFile(filename, modes);
        }
    }

    else if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = SaveFileList::openSaveFile(filename, modes);
    }

    else {
        f = orig::fopen64(filename, modes);
    }

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

    /* Check for urandom */
    if (urandom_get_file() == stream) {
        return 0;
    }

    /* Check if we must actually close the file */
    bool doClose = FileHandleList::closeFile(fileno(stream));

    if (doClose) {
        int ret = SaveFileList::closeSaveFile(stream);
        if (ret != 1)
            return ret;

        return orig::fclose(stream);
    }

    return 0;
}

// int fileno (FILE *stream) __THROW
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
