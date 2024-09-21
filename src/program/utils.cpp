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

#include "utils.h"
#include "Context.h"

#include <sys/stat.h>
#include <cerrno> // errno
#include <cstring> // strerror
#include <iostream>
#include <unistd.h> // unlink

std::string fileFromPath(const std::string& path)
{
    size_t sep = path.find_last_of("/");
    if (sep != std::string::npos)
        return path.substr(sep + 1);
    else
        return path;
}

std::string dirFromPath(const std::string& path)
{
    size_t sep = path.find_last_of("/");
    if (sep != std::string::npos)
        return path.substr(0, sep);
    else
        return "";
}

std::string realpath_nonexist(const std::string& path)
{
    std::string absstr;

    char* abspath = realpath(path.c_str(), nullptr);
    if (abspath) {
        absstr = abspath;
        free(abspath);
        return absstr;
    }

    if (errno == ENOENT) {
        /* If file does not exist, get the absolute path of the directory and
         * append the file. */

        std::string dir;
        std::string file;

        size_t sep = path.find_last_of("/");
        if (sep != std::string::npos) {
            dir = path.substr(0, sep);
            file = path.substr(sep+1);
        }
        else {
            dir = ".";
            file = path;
        }

        abspath = realpath(dir.c_str(), nullptr);
        if (abspath) {
            absstr = abspath;
            absstr += "/";
            absstr += file;
            free(abspath);
            return absstr;
        }
    }

    return absstr;
}


int create_dir(const std::string& path)
{
    int ret = mkdir(path.c_str(), S_IRWXU);
    if (ret == 0)
        return 0;

    switch (errno) {
    case ENOENT:
        {
            /* parent didn't exist, try to create it */
            std::string dir = dirFromPath(path);
            if (dir.empty())
                return -1;
            else if (create_dir(dir) == -1)
                return -1;

            return mkdir(path.c_str(), S_IRWXU);
        }

    case EEXIST:
        {
            struct stat sb;
            stat(path.c_str(), &sb);
            if (!S_ISDIR(sb.st_mode))
            {
                std::cerr << "Something has the same name as the directory to be created: " << path << std::endl;
                return -1;
            }
            return 0;
        }
    default:
        std::cerr << "The following directory could not be created: " << path << std::endl;
        return -1;
    }

    return 0;
}

void remove_savestates(Context* context)
{
    std::string savestateprefix = context->config.savestatedir + '/';
    savestateprefix += context->gamename;
    for (int i=0; i<=10; i++) {
        std::string savestatepmpath = savestateprefix + ".state" + std::to_string(i) + ".pm";
        unlink(savestatepmpath.c_str());
        std::string savestatepspath = savestateprefix + ".state" + std::to_string(i) + ".p";
        unlink(savestatepspath.c_str());
    }
}

int extractBinaryType(std::string path)
{
    int extra_flags = 0;
    
    /* Check for MacOS app file, and extract the actual executable if so. */
    std::string executable_path = extractMacOSExecutable(path);
    if (!executable_path.empty()) {
        path = executable_path;
        extra_flags = BT_MACOSAPP;
    }
    
    std::string cmd = "file --brief --dereference \"";
    cmd += path;
    cmd += "\"";

    std::string outputstr = queryCmd(cmd);

    if (outputstr.find("pie executable") != std::string::npos) {
        extra_flags |= BT_PIEAPP;
    }

    if (outputstr.find("ELF 32-bit") != std::string::npos) {
        return BT_ELF32 | extra_flags;
    }

    if (outputstr.find("ELF 64-bit") != std::string::npos) {
        return BT_ELF64 | extra_flags;
    }

    if (outputstr.find("PE32 executable") != std::string::npos) {
        return BT_PE32 | extra_flags;
    }

    if (outputstr.find("PE32+ executable") != std::string::npos) {
        return BT_PE32P | extra_flags;
    }

    if (outputstr.find("MS-DOS executable, NE") != std::string::npos) {
        return BT_NE | extra_flags;
    }

    if (outputstr.find("Bourne-Again shell script") != std::string::npos) {
        return BT_SH | extra_flags;
    }

    if (outputstr.find("Mach-O universal binary") != std::string::npos) {
        return BT_MACOSUNI | extra_flags;
    }

    if (outputstr.find("Mach-O executable i386") != std::string::npos) {
        return BT_MACOS32 | extra_flags;
    }

    if (outputstr.find("Mach-O 64-bit") != std::string::npos) {
        return BT_MACOS64 | extra_flags;
    }

    return BT_UNKNOWN | extra_flags;
}

std::string extractMacOSExecutable(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        /* Extract file name and check for '.app' extension */
        std::string name = fileFromPath(path);
        if (name.substr(name.find_last_of(".") + 1) != "app")
            return "";
        
        /* Get executable name from Info.plist CFBundleExecutable field */
        std::string plist_cmd = "defaults read \"";
        plist_cmd += path;
        plist_cmd += "/Contents/Info.plist\" CFBundleExecutable";
        
        std::string outputstr = queryCmd(plist_cmd);

        /* Build path to executable */
        std::string executable_path = path;
        executable_path += "/Contents/MacOS/";
        executable_path += outputstr;

        /* Check that the file exists */
        if (access(executable_path.c_str(), F_OK) != 0)
            return "";

        return executable_path;
    }

    return "";
}

std::string queryCmd(const std::string& cmd, int* status)
{
    std::string outputstr;
    FILE *output = popen(cmd.c_str(), "r");
    if (output != NULL) {
        char buf[256];
        if (fgets(buf, 256, output) != 0) {
            outputstr = buf;
        }
        int s = pclose(output);
        if (status) *status = s;
    }

    /* Trim the value */
    size_t end = outputstr.find_last_not_of(" \n\r\t\f\v");
    outputstr = (end == std::string::npos) ? "" : outputstr.substr(0, end + 1);
    return outputstr;
}
