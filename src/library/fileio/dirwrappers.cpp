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

#include "dirwrappers.h"

#include "../logging.h"
#include "../hook.h"
#include "SaveFileList.h"
// #include "FileHandleList.h"
#include "../GlobalState.h"
// #include "URandom.h"

#include <unistd.h> // readlink

namespace libtas {

DEFINE_ORIG_POINTER(opendir)
DEFINE_ORIG_POINTER(fdopendir)
DEFINE_ORIG_POINTER(closedir)
DEFINE_ORIG_POINTER(readdir)
DEFINE_ORIG_POINTER(readdir64)
DEFINE_ORIG_POINTER(readdir_r)
DEFINE_ORIG_POINTER(readdir64_r)

#define DIROFF_SIZE 10
static DIR *dird[DIROFF_SIZE];
static int diri[DIROFF_SIZE];
static std::string dirpath[DIROFF_SIZE];

DIR *opendir (const char *name)
{
    LINK_NAMESPACE_GLOBAL(opendir);

    DIR *d = orig::opendir(name);

    if (GlobalState::isNative())
        return d;

    debuglogstdio(LCF_FILEIO, "%s call with dir %s", __func__, name);

    if (!shared_config.prevent_savefiles)
        return d;

    /* Register internal offset of dir d */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == 0) {
            dird[i] = d;
            diri[i] = 0;
            dirpath[i] = name;
            break;
        }
    }
    if (i == DIROFF_SIZE) {
        debuglogstdio(LCF_FILEIO | LCF_ERROR, "   could not register dir, not enough space");
    }
    
    return d;
}

DIR *fdopendir (int fd)
{
    LINK_NAMESPACE_GLOBAL(fdopendir);

    DIR *d = orig::fdopendir(fd);

    if (GlobalState::isNative())
        return d;

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);

    if (!shared_config.prevent_savefiles)
        return d;

    /* Register internal offset of dir d */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == 0) {
            dird[i] = d;
            diri[i] = 0;
            
            /* Get file path from fd */
            std::string link = "/proc/self/fd/";
            link += std::to_string(fd);
            char buf[4096] = {0};
            readlink(link.c_str(), buf, 4096);
            if (buf[0] == '/') {
                dirpath[i] = buf;            
            }
            else {
                debuglogstdio(LCF_FILEIO | LCF_ERROR, "   could not get path from fd");
                dird[i] = 0;
            }
            break;
        }
    }
    if (i == DIROFF_SIZE) {
        debuglogstdio(LCF_FILEIO | LCF_ERROR, "   could not register dir, not enough space");
    }
    
    return d;    
}

int closedir (DIR *dirp)
{
    LINK_NAMESPACE_GLOBAL(closedir);

    int ret = orig::closedir(dirp);

    if (GlobalState::isNative())
        return ret;

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (!shared_config.prevent_savefiles)
        return ret;

    /* Unregister internal offset of dir d */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp) {
            dird[i] = 0;
            diri[i] = 0;
            break;
        }
    }
    if (i == DIROFF_SIZE && ret == 0) {
        debuglogstdio(LCF_FILEIO | LCF_ERROR, "   could not unregister dir");
    }
    
    return ret;
}

struct dirent *readdir (DIR *dirp)
{
    static struct dirent dir;
    
    LINK_NAMESPACE_GLOBAL(readdir);

    if (GlobalState::isNative())
        return orig::readdir(dirp);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (!shared_config.prevent_savefiles)
        return orig::readdir(dirp);

    /* First, list all savefiles from directory */
    for (int i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp) {

            /* We already browse all savefiles from this directory */
            if (diri[i] == -1)
                break;

            /* Get savefile inside dir */
            std::string filename = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);

            if (!filename.empty()) {
                /* Get file or directory from relative path */
                size_t sep = filename.find_first_of("/");
                if (sep != std::string::npos) {
                    filename = filename.substr(0, sep);
                    dir.d_type = DT_DIR;
                }
                else {
                    dir.d_type = DT_REG;                    
                }

                strncpy(dir.d_name, filename.c_str(), 255);
                diri[i]++;
                return &dir;
            }
            else {
                /* Set an invalid index, so that future calls won't check for that */
                diri[i] = -1;
            }
            break;
        }
    }
    
    /* Then, list all actual files */
    return orig::readdir(dirp);
}

struct dirent64 *readdir64 (DIR *dirp)
{
    static struct dirent64 dir;
    
    LINK_NAMESPACE_GLOBAL(readdir64);

    if (GlobalState::isNative())
        return orig::readdir64(dirp);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (!shared_config.prevent_savefiles)
        return orig::readdir64(dirp); 

    /* First, list all savefiles from directory */
    for (int i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp) {

            /* We already browse all savefiles from this directory */
            if (diri[i] == -1)
                break;

            /* Get savefile inside dir */
            std::string filename = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);

            if (!filename.empty()) {
                /* Get file or directory from relative path */
                size_t sep = filename.find_first_of("/");
                if (sep != std::string::npos) {
                    filename = filename.substr(0, sep);
                    dir.d_type = DT_DIR;
                }
                else {
                    dir.d_type = DT_REG;                    
                }

                strncpy(dir.d_name, filename.c_str(), 255);
                diri[i]++;
                return &dir;
            }
            else {
                /* Set an invalid index, so that future calls won't check for that */
                diri[i] = -1;
            }
            break;
        }
    }
    
    /* Then, list all actual files */
    return orig::readdir64(dirp);
}

int readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result)
{
    LINK_NAMESPACE_GLOBAL(readdir_r);

    if (GlobalState::isNative())
        return orig::readdir_r(dirp, entry, result);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (!shared_config.prevent_savefiles)
        return orig::readdir_r(dirp, entry, result);

    /* First, list all savefiles from directory */
    for (int i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp) {

            /* We already browse all savefiles from this directory */
            if (diri[i] == -1)
                break;

            /* Get savefile inside dir */
            std::string filename = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);

            if (!filename.empty()) {
                /* Get file or directory from relative path */
                size_t sep = filename.find_first_of("/");
                if (sep != std::string::npos) {
                    filename = filename.substr(0, sep);
                    entry->d_type = DT_DIR;
                }
                else {
                    entry->d_type = DT_REG;                    
                }
                
                strncpy(entry->d_name, filename.c_str(), 255);
                *result = entry;
                diri[i]++;
                debuglogstdio(LCF_FILEIO, "   return savefile %s", entry->d_name);
                return 0;
            }
            else {
                /* Set an invalid index, so that future calls won't check for that */
                diri[i] = -1;
            }
            break;
        }
    }
    
    /* Then, list all actual files */
    return orig::readdir_r(dirp, entry, result);
}

int readdir64_r (DIR *dirp, struct dirent64 *entry, struct dirent64 **result)
{
    LINK_NAMESPACE_GLOBAL(readdir64_r);

    if (GlobalState::isNative())
        return orig::readdir64_r(dirp, entry, result);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (!shared_config.prevent_savefiles)
        return orig::readdir64_r(dirp, entry, result); 

    /* First, list all savefiles from directory */
    for (int i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp) {

            /* We already browse all savefiles from this directory */
            if (diri[i] == -1)
                break;

            /* Get savefile inside dir */
            std::string filename = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);

            if (!filename.empty()) {
                /* Get file or directory from relative path */
                size_t sep = filename.find_first_of("/");
                if (sep != std::string::npos) {
                    filename = filename.substr(0, sep);
                    entry->d_type = DT_DIR;
                }
                else {
                    entry->d_type = DT_REG;                    
                }

                strncpy(entry->d_name, filename.c_str(), 255);
                *result = entry;
                diri[i]++;
                return 0;
            }
            else {
                /* Set an invalid index, so that future calls won't check for that */
                diri[i] = -1;
            }
            break;
        }
    }
    
    /* Then, list all actual files */
    return orig::readdir64_r(dirp, entry, result);
}


}
