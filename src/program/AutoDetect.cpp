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

#include "AutoDetect.h"
#include "utils.h"
#include "movie/MovieFile.h"
#include "Context.h"

#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h> // access()

int AutoDetect::arch(Context *context)
{
    /* Change settings based on game arch */
    int gameArch = extractBinaryType(context->gamepath);
    int libtasArch = extractBinaryType(context->libtaspath);

    /* Switch to libtas32.so if required */
    if ((((gameArch&BT_TYPEMASK) == BT_ELF32) || ((gameArch&BT_TYPEMASK) == BT_PE32) || ((gameArch&BT_TYPEMASK) == BT_NE)) && (libtasArch == BT_ELF64)) {
        context->libtaspath = context->libtas32path;
        /* libtas32.so presence was already checked in ui/ErrorChecking.cpp */
        libtasArch = extractBinaryType(context->libtaspath);
    }

    return gameArch;
}

void AutoDetect::game_libraries(Context *context)
{
    /* Build a command to parse the first missing library from the game executable,
     * look at game directory and sub-directories for it */
    std::ostringstream oss_ml;
    oss_ml << "ldd '" << context->gamepath;
    oss_ml << "' | awk '/ => not found/ { print $1 }' | head -1";
    
    std::string missing_lib = queryCmd(oss_ml.str());
    if (missing_lib.empty()) return;
    
    std::cout << "Try to find the location of " << missing_lib << " among game files."<< std::endl;

    std::string gamedir = dirFromPath(context->gamepath);
    std::ostringstream oss_lp;
    oss_lp << "find '" << gamedir << "' -name " << missing_lib << " -type f -print -quit";

    std::string found_lib = queryCmd(oss_lp.str());
    if (!found_lib.empty()) {
        std::cout << "-> library was found at location " << found_lib << std::endl;
        
        std::string found_lib_dir = dirFromPath(found_lib);

        char* oldlibpath = getenv("LD_LIBRARY_PATH");
        if (oldlibpath) {
            found_lib_dir.append(":");
            found_lib_dir.append(oldlibpath);
        }
        setenv("LD_LIBRARY_PATH", found_lib_dir.c_str(), 1);
    }
    else {
        std::cerr << "-> could not find the library among the game files" << std::endl;
    }
    
    /* Try to download common missing libraries */
    if (context->config.allow_downloads != 1)
        return;
    
    int gameArch = extractBinaryType(context->gamepath);
    if (gameArch != BT_ELF32 && gameArch != BT_ELF64)
        return;
    
    missing_lib = queryCmd(oss_ml.str());

    while (! missing_lib.empty()) {
        std::string libUrl, libDeb, libStr;
        if (missing_lib == "libcrypto.so.1.0.0") {
            if (gameArch == BT_ELF32) {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openssl1.0/libssl1.0.0_1.0.2n-1ubuntu5.13_i386.deb";
            }
            else {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openssl1.0/libssl1.0.0_1.0.2n-1ubuntu5.13_amd64.deb";
            }
        }
        else if (missing_lib == "libssl.so.1.0.0") {
            if (gameArch == BT_ELF32) {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openssl1.0/libssl1.0.0_1.0.2n-1ubuntu5.13_i386.deb";
            }
            else {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openssl1.0/libssl1.0.0_1.0.2n-1ubuntu5.13_amd64.deb";
            }
        }
        else if (missing_lib == "libldap-2.4.so.2") {
            if (gameArch == BT_ELF32) {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openldap/libldap-2.4-2_2.4.49+dfsg-2ubuntu1.10_i386.deb";
            }
            else {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openldap/libldap-2.4-2_2.4.49+dfsg-2ubuntu1.10_amd64.deb";
            }
        }
        else if (missing_lib == "liblber-2.4.so.2") {
            if (gameArch == BT_ELF32) {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openldap/libldap-2.4-2_2.4.49+dfsg-2ubuntu1.10_i386.deb";
            }
            else {
                libUrl = "http://security.ubuntu.com/ubuntu/pool/main/o/openldap/libldap-2.4-2_2.4.49+dfsg-2ubuntu1.10_amd64.deb";
            }
        }
        else {
            return;
        }
        
        std::cerr << "Library " << missing_lib << " is missing. Downloading it from " << libUrl << std::endl;

        std::ostringstream oss_lib;
        oss_lib << "wget -nc -P ${TMPDIR:-/tmp} " << libUrl;
        oss_lib << " && ar -x --output ${TMPDIR:-/tmp} ${TMPDIR:-/tmp}/" << fileFromPath(libUrl) << " data.tar.xz";
        
        std::string libFile = (gameArch == BT_ELF32) ? "./usr/lib/i386-linux-gnu/" : "./usr/lib/x86_64-linux-gnu/";
        
        oss_lib << " && tar -xf ${TMPDIR:-/tmp}/data.tar.xz -C ${TMPDIR:-/tmp} " << libFile;
        
        int status;
        queryCmd(oss_lib.str(), &status);

        if (status != 0) {
            std::cerr << "Downloading failed..." << std::endl;
            return;
        }

        /* The extracted library may be a symlink to another library file, so
         * move all the symlink chain to the real file */
        std::string current_lib = missing_lib;
        
        while (!current_lib.empty()) {
            /* Get the symlink if any, or it returns empty string */
            std::ostringstream oss_sym;
            oss_sym << "readlink ${TMPDIR:-/tmp}/" << libFile << current_lib;
            
            std::string symlink_lib = queryCmd(oss_sym.str());
            
            /* Move the library file */
            std::ostringstream oss_move;
            oss_move << "mv ${TMPDIR:-/tmp}/" << libFile << current_lib << " ";
            oss_move << ((gameArch == BT_ELF32) ? context->config.extralib32dir : context->config.extralib64dir);
            
            queryCmd(oss_move.str());
            
            current_lib = symlink_lib;
        }

        /* Check if for some reason, adding the library still shows as missing,
         * to prevent a potential softlock */
        std::string old_missing_lib = missing_lib;
        missing_lib = queryCmd(oss_ml.str());
        
        if (old_missing_lib == missing_lib) {
            std::cerr << "Loading library " << missing_lib << " did not work, exiting." << std::endl;
            return;
        }
    }
}

void AutoDetect::game_engine(Context *context)
{
    struct stat sb;
    
    /* Check for Unity:
     * Unity executables end with `.x86` or `.x86_64`, and data directory is named
     * after the executable with added `_Data` */
    size_t pos = context->gamepath.find_last_of(".");
    std::string ext;
    if (pos != std::string::npos)
        ext = context->gamepath.substr(pos);
    
    if (ext == ".x86" || ext == ".x86_64") {
        std::string data = context->gamepath.substr(0, pos) + "_Data";

        if (stat(data.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
            std::cout << "Unity game detected" << std::endl;
            std::ostringstream oss;
            oss << "strings '" << data;
            oss << "/Resources/unity_builtin_extra' | head -n 1";

            std::string version = queryCmd(oss.str());
            if (!version.empty())
                std::cout << "   Version " << version << std::endl;
            else
                std::cout << "   Could not detect version " << std::endl;

            /* Check for -force-gfx-direct command-line option */
            if (context->config.gameargs.find("-force-gfx-direct") == std::string::npos) {
                std::cout << "   Adding -force-gfx-direct command-line option" << std::endl;
                context->config.gameargs += " -force-gfx-direct";
            }
            
            return;
        }
    }
    
    /* Check for GM:S:
     * Games have an asset folder which contains the `game.unx` file */
    std::string assets = dirFromPath(context->gamepath) + "/assets";
    if (stat(assets.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        std::string gameunx = assets + "/game.unx";
        if (access(gameunx.c_str(), F_OK) == 0) {
            std::cout << "GameMaker Studio game detected" << std::endl;

            /* Check time-tracking clockgettime monotonic */
            if (context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] == -1) {
                std::cout << "   Adding time-tracking clock_gettime() monotonic" << std::endl;                
                context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] = 100;
            }
            return;
        }
    }

    /* Check for Godot:
     * Look at symbols inside the game executable and count `godot_*` */
    std::ostringstream oss;
    oss << "readelf -WsC '" << context->gamepath;
    oss << "' | grep godot_ | wc -l ";

    std::string godotcount = queryCmd(oss.str());
    if (!godotcount.empty() && (std::strtoul(godotcount.c_str(), nullptr, 10) > 100)) {
        std::cout << "Godot game detected" << std::endl;

        /* Check for --audio-driver command-line option */
        if (context->config.gameargs.find("--audio-driver") == std::string::npos) {
            std::cout << "   Adding --audio-driver ALSA command-line option" << std::endl;
            context->config.gameargs += " --audio-driver ALSA";
        }
    }
}
