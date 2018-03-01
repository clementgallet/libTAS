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

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "../logging.h"
#include "../hook.h"
#include "detectsavefiles.h"
#include <map>
#include <string>
#include <sys/stat.h>
#include "../GlobalState.h"

namespace libtas {

/*** Helper functions ***/

static std::map<std::string,std::pair<char*,size_t>> savefile_buffers;

/*
 * Create a memory stream with a copy of the content of the file using
 * open_memstream. Save the base pointer and size into a map because we may
 * need them again if the game open the file again in read or write mode.
 * If we already opened this file, don't copy the original content of the file
 * but the content of the old memory buffer.
 */
static FILE* get_memstream(const char* source, const char* modes)
{
    std::string sstr(source);
    FILE* memstream;

    /* Do we need to overwrite the content of the file ? */
    bool overwrite = (strstr(modes, "w") != nullptr);

    /*
     * If we already register the savefile, we must copy the content of the
     * old buffer to the new stream.
     */
    if (savefile_buffers.find(sstr) != savefile_buffers.end()) {

        /* Open a new memory stream using pointers to the previous memory buffer
         * and size. Pointers are not updated until fflush or fclose, so we
         * still have access to the old buffer.
         */
        memstream = open_memstream(&savefile_buffers[sstr].first, &savefile_buffers[sstr].second);

        if (!overwrite) {
            /* Append the content of the old buffer to the stream */
            fwrite(savefile_buffers[sstr].first, 1, savefile_buffers[sstr].second, memstream);
        }

        /* Free the old buffer */
        free(savefile_buffers[sstr].first);

    }
    else {
        /* Create an entry in our map */
        savefile_buffers[sstr].first = 0;
        savefile_buffers[sstr].second = 0;

        /* Open a new memory stream using pointers to these entries */
        memstream = open_memstream(&savefile_buffers[sstr].first, &savefile_buffers[sstr].second);

        if (!overwrite) {
            /* Append the content of the file to the stream if the file exists */
            struct stat filestat;
            int rv = stat(source, &filestat);

            if (rv == 0) {
                /* The file exists, copying the content to the stream */
                GlobalNative gn;

                FILE* f = fopen(source, "rb");

                if (f != nullptr) {
                    char tmp_buf[4096];
                    size_t s;
                    do {
                        s = fread(tmp_buf, 1, 4096, f);
                        fwrite(tmp_buf, 1, s, memstream);
                    } while(s != 0);

                    fclose(f);
                }
            }
        }
    }

    /* If not in append mode, seek to the beginning of the stream */
    if (strstr(modes, "a") == nullptr)
        fseek(memstream, 0, SEEK_SET);

    return memstream;
}

/* Specific savefile check for stdio and SDL open functions */
static bool isSaveFile(const char *file, const char *modes)
{
    static bool inited = 0;
    if (!inited) {
        /*
         * Normally, we shouldn't have to clear the savefiles set,
         * as it is clearly during creation. However, games break without
         * clearing it. I suppose it is because we are using the set
         * before it had time to initialize, and it seems clearing it
         * is enough to make it usable.
         */
        savefile_buffers.clear();
        inited = 1;
    }

    /* If the file has already been registered as a savefile, open our memory
     * buffer, even if the open is read-only.
     */
    std::string sstr(file);
    if (savefile_buffers.find(sstr) != savefile_buffers.end())
        return true;

    /* If the file was not registered, check if the opening is writeable. */
    if (!isWriteable(modes))
        return false;

    /* Check the file is regular */
    return isSaveFile(file);
}

namespace orig {
    static FILE *(*fopen) (const char *filename, const char *modes) = nullptr;
    static FILE *(*fopen64) (const char *filename, const char *modes) = nullptr;
    static int (*fclose) (FILE *stream) = nullptr;
}

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

    if (!GlobalState::isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = get_memstream(filename, modes);
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

    if (!GlobalState::isOwnCode() && isSaveFile(filename, modes)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        f = get_memstream(filename, modes);
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

    int rv = orig::fclose(stream);

    return rv;
}

}

#endif
