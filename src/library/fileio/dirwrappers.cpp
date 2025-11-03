/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "SaveFile.h"
#include "SaveFileList.h"

#include "logging.h"
#include "hook.h"
#include "global.h"
#include "GlobalState.h"

#include <unistd.h> // readlink

namespace libtas {

#ifdef __unix__
DEFINE_ORIG_POINTER(opendir)
DEFINE_ORIG_POINTER(fdopendir)
DEFINE_ORIG_POINTER(closedir)
DEFINE_ORIG_POINTER(readdir)
DEFINE_ORIG_POINTER(readdir_r)
DEFINE_ORIG_POINTER(readdir64)
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

    LOG(LL_TRACE, LCF_FILEIO, "%s call with dir %s", __func__, name);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return d;

    if (!Global::shared_config.prevent_savefiles)
        return d;

    /* Register internal offset of dir d */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == 0) {
            dird[i] = d;
            diri[i] = 0;
            dirpath[i] = name;
            if (dirpath[i].back() != '/')
                dirpath[i].push_back('/');
            break;
        }
    }
    if (i == DIROFF_SIZE) {
        LOG(LL_ERROR, LCF_FILEIO, "   could not register dir, not enough space");
    }
    
    return d;
}

DIR *fdopendir (int fd)
{
    LINK_NAMESPACE_GLOBAL(fdopendir);

    DIR *d = orig::fdopendir(fd);

    if (GlobalState::isNative())
        return d;

    LOG(LL_TRACE, LCF_FILEIO, "%s call with fd %d", __func__, fd);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return d;

    if (!Global::shared_config.prevent_savefiles)
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
                LOG(LL_ERROR, LCF_FILEIO, "   could not get path from fd");
                dird[i] = 0;
            }
            break;
        }
    }
    if (i == DIROFF_SIZE) {
        LOG(LL_ERROR, LCF_FILEIO, "   could not register dir, not enough space");
    }
    
    return d;    
}

int closedir (DIR *dirp)
{
    LINK_NAMESPACE_GLOBAL(closedir);

    int ret = orig::closedir(dirp);

    if (GlobalState::isNative())
        return ret;

    LOG(LL_TRACE, LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return ret;

    if (!Global::shared_config.prevent_savefiles)
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
        LOG(LL_ERROR, LCF_FILEIO, "   could not unregister dir");
    }
    
    return ret;
}

struct dirent *readdir (DIR *dirp)
{
    static struct dirent dir;
    
    LINK_NAMESPACE_GLOBAL(readdir);

    if (GlobalState::isNative())
        return orig::readdir(dirp);

    LOG(LL_TRACE, LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::readdir(dirp);

    if (!Global::shared_config.prevent_savefiles)
        return orig::readdir(dirp);

    /* Recover the informations from the opened dir */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp)
            break;
    }
    
    /* First, list all savefiles from directory */

    /* Check if we already browse all savefiles from this directory */
    if (diri[i] != -1) {
        /* Get savefile inside dir */
        for (const SaveFile* savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);
        savefile != nullptr; savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i])) {

            diri[i]++;
            
            /* Don't show savefiles that were removed */
            if (savefile->removed) {
                LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s that was removed", savefile->filename.c_str());
                continue;
            }
            
            /* Get file or directory from relative path */
            std::string filename = savefile->filename.substr(dirpath[i].size());
            
            size_t sep = filename.find_first_of("/");
            if (sep != std::string::npos) {
                filename = filename.substr(0, sep);
                dir.d_type = DT_DIR;
            }
            else {
                dir.d_type = DT_REG;
            }
            
            strncpy(dir.d_name, filename.c_str(), 255);
            LOG(LL_DEBUG, LCF_FILEIO, "   return savefile %s", dir.d_name);
            return &dir;
        }
    }

    /* Set an invalid index, so that future calls won't check for that */
    diri[i] = -1;
    
    /* Then, list all actual files, without savefiles */
    for (struct dirent* d = orig::readdir(dirp); d; d = orig::readdir(dirp)) {
        /* Build the full path and check if savefile */
        std::string filepath = dirpath[i];
        if (filepath.back() != '/')
            filepath.push_back('/');
        filepath += d->d_name;

        if (!SaveFileList::getSaveFile(filepath.c_str()))
            return d;
        else
            LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s", d->d_name);
    }
    return nullptr;
}

#ifdef __unix__
struct dirent64 *readdir64 (DIR *dirp)
{
    static struct dirent64 dir;

    LINK_NAMESPACE_GLOBAL(readdir64);

    if (GlobalState::isNative())
        return orig::readdir64(dirp);

    LOG(LL_TRACE, LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::readdir64(dirp);

    if (!Global::shared_config.prevent_savefiles)
        return orig::readdir64(dirp); 

    /* Recover the informations from the opened dir */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp)
            break;
    }

    /* First, list all savefiles from directory */

    /* Check if we already browse all savefiles from this directory */
    if (diri[i] != -1) {
        /* Get savefile inside dir */
        for (const SaveFile* savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);
        savefile != nullptr; savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i])) {

            diri[i]++;
            
            /* Don't show savefiles that were removed */
            if (savefile->removed) {
                LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s that was removed", savefile->filename.c_str());
                continue;
            }
            
            /* Get file or directory from relative path */
            std::string filename = savefile->filename.substr(dirpath[i].size());
            
            size_t sep = filename.find_first_of("/");
            if (sep != std::string::npos) {
                filename = filename.substr(0, sep);
                dir.d_type = DT_DIR;
            }
            else {
                dir.d_type = DT_REG;
            }
            
            strncpy(dir.d_name, filename.c_str(), 255);
            LOG(LL_DEBUG, LCF_FILEIO, "   return savefile %s", dir.d_name);
            return &dir;
        }
    }

    /* Set an invalid index, so that future calls won't check for that */
    diri[i] = -1;
    
    /* Then, list all actual files, without savefiles */
    for (struct dirent64* d = orig::readdir64(dirp); d; d = orig::readdir64(dirp)) {
        /* Build the full path and check if savefile */
        std::string filepath = dirpath[i];
        if (filepath.back() != '/')
            filepath.push_back('/');
        filepath += d->d_name;

        if (!SaveFileList::getSaveFile(filepath.c_str()))
            return d;
        else
            LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s", d->d_name);
    }
    return nullptr;
}
#endif

int readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result)
{
    LINK_NAMESPACE_GLOBAL(readdir_r);

    if (GlobalState::isNative())
        return orig::readdir_r(dirp, entry, result);

    LOG(LL_TRACE, LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::readdir_r(dirp, entry, result);

    if (!Global::shared_config.prevent_savefiles)
        return orig::readdir_r(dirp, entry, result);

    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp)
            break;
    }
    
    /* First, list all savefiles from directory */

    /* Check if we already browse all savefiles from this directory */
    if (diri[i] != -1) {
        /* Get savefile inside dir */
        for (const SaveFile* savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);
        savefile != nullptr; savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i])) {
            diri[i]++;

            /* Don't show savefiles that were removed */
            if (savefile->removed) {
                LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s that was removed", savefile->filename.c_str());
                continue;
            }
            
            /* Get file or directory from relative path */
            LOG(LL_DEBUG, LCF_FILEIO, "   found savefile %s", savefile->filename.c_str());
            std::string filename = savefile->filename.substr(dirpath[i].size());
            LOG(LL_DEBUG, LCF_FILEIO, "   found savefile file %s", filename.c_str());
            
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
            LOG(LL_DEBUG, LCF_FILEIO, "   return savefile %s", entry->d_name);
            return 0;
        }
    }

    /* Set an invalid index, so that future calls won't check for that */
    diri[i] = -1;
    
    /* Then, list all actual files, without savefiles */
    for (int res = orig::readdir_r(dirp, entry, result); (*result != nullptr); res = orig::readdir_r(dirp, entry, result)) {
        /* Build the full path and check if savefile */
        std::string filepath = dirpath[i];
        filepath += entry->d_name;

        if (!SaveFileList::getSaveFile(filepath.c_str()))
            return res;
        else
            LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s", entry->d_name);
    }
    return 0;
}

int readdir64_r (DIR *dirp, struct dirent64 *entry, struct dirent64 **result)
{
    LINK_NAMESPACE_GLOBAL(readdir64_r);

    if (GlobalState::isNative())
        return orig::readdir64_r(dirp, entry, result);

    LOG(LL_TRACE, LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::readdir64_r(dirp, entry, result); 

    if (!Global::shared_config.prevent_savefiles)
        return orig::readdir64_r(dirp, entry, result); 

    /* Recover the informations from the opened dir */
    int i;
    for (i = 0; i < DIROFF_SIZE; i++) {
        if (dird[i] == dirp)
            break;
    }

    /* First, list all savefiles from directory */

    /* Check if we already browse all savefiles from this directory */
    if (diri[i] != -1) {
        /* Get savefile inside dir */
        for (const SaveFile* savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i]);
        savefile != nullptr; savefile = SaveFileList::getSaveFileInsideDir(dirpath[i], diri[i])) {
            diri[i]++;

            /* Don't show savefiles that were removed */
            if (savefile->removed) {
                LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s that was removed", savefile->filename.c_str());
                continue;
            }
            
            /* Get file or directory from relative path */
            LOG(LL_DEBUG, LCF_FILEIO, "   found savefile %s", savefile->filename.c_str());
            std::string filename = savefile->filename.substr(dirpath[i].size());
            LOG(LL_DEBUG, LCF_FILEIO, "   found savefile file %s", filename.c_str());
            
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
            LOG(LL_DEBUG, LCF_FILEIO, "   return savefile %s", entry->d_name);
            return 0;
        }
    }

    /* Set an invalid index, so that future calls won't check for that */
    diri[i] = -1;
    
    /* Then, list all actual files, without savefiles */
    for (int res = orig::readdir64_r(dirp, entry, result); (*result != nullptr); res = orig::readdir64_r(dirp, entry, result)) {
        /* Build the full path and check if savefile */
        std::string filepath = dirpath[i];
        filepath += entry->d_name;

        if (!SaveFileList::getSaveFile(filepath.c_str()))
            return res;
        else
            LOG(LL_DEBUG, LCF_FILEIO, "   skip savefile %s", entry->d_name);
    }
    return 0;
}
#endif

}
