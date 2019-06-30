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

#include <QMessageBox>

#include "ErrorChecking.h"
#include "../utils.h"
#include <unistd.h>
#include <sys/stat.h>

bool ErrorChecking::allChecks(Context* context)
{
    if (!checkGameExists(context->gamepath))
        return false;

    if (context->config.sc.recording == SharedConfig::RECORDING_READ)
        if (!checkMovieExists(context->config.moviefile))
            return false;

    if (context->config.sc.recording == SharedConfig::RECORDING_WRITE)
        if (!checkMovieWriteable(context->config.moviefile))
            return false;

    if (!checkArchType(context))
        return false;

    return true;
}

bool ErrorChecking::checkGameExists(std::string gamepath)
{
    /* Checking that the game binary exists */
    if (access(gamepath.c_str(), F_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("Game path %1 was not found").arg(gamepath.c_str()));
        return false;
    }

    return true;
}

bool ErrorChecking::checkMovieExists(std::string moviepath)
{
    /* Checking that the movie file exists */
    if (access(moviepath.c_str(), F_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("Movie path %1 was not found").arg(moviepath.c_str()));
        return false;
    }

    return true;
}

bool ErrorChecking::checkMovieWriteable(std::string moviepath)
{
    /* Extract the movie directory */
    std::string moviedir;
    size_t sep = moviepath.find_last_of("/");
    if (sep != std::string::npos)
        moviedir = moviepath.substr(0, sep);
    else {
        QMessageBox::critical(nullptr, "Error", QString("The movie path %1 is not an absolute path").arg(moviedir.c_str()));
        return false;
    }

    /* Check that the directory of the moviefile exists */
    struct stat sb;
    if (stat(moviedir.c_str(), &sb) == -1) {
        if (errno == ENOENT) {
            /* The directory does not exist */
            QMessageBox::critical(nullptr, "Error", QString("The directory of the moviefile %1 does not exists").arg(moviedir.c_str()));
            return false;
        }
        else {
            /* Another error */
            QMessageBox::critical(nullptr, "Error", QString("The directory of the moviefile %1 cannot be accessed").arg(moviedir.c_str()));
            return false;
        }
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        QMessageBox::critical(nullptr, "Error", QString("The directory of the moviefile %1 is not a directory").arg(moviedir.c_str()));
        return false;
    }

    /* Checking that the user can create a movie file */
    if (access(moviedir.c_str(), W_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("You don't have permission to create moviefile %1").arg(moviepath.c_str()));
        return false;
    }

    /* Prompt a confirmation message if overwriting a movie file */
    if (access(moviepath.c_str(), F_OK) == 0) {
        QMessageBox::StandardButton btn = QMessageBox::question(nullptr, "Movie overwrite", QString("The movie file %1 does exist. Do you want to overwrite it?").arg(moviepath.c_str()), QMessageBox::Ok | QMessageBox::Cancel);
        if (btn != QMessageBox::Ok)
            return false;
    }

    return true;
}

bool ErrorChecking::checkArchType(Context* context)
{
    /* Checking that the game binary exists (again) */
    if (access(context->gamepath.c_str(), F_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("Game path %1 was not found").arg(context->gamepath.c_str()));
        return false;
    }

    /* Checking that the libtas.so library path is correct */
    if (access(context->libtaspath.c_str(), F_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("libtas.so library at %1 was not found. Make sure that the file libtas.so is in the same directory as libTAS file").arg(context->libtaspath.c_str()));
        return false;
    }

    int gameArch = extractBinaryType(context->gamepath);
    if (gameArch <= 0) {
        QMessageBox::critical(nullptr, "Error", QString("Could not determine arch of file %1").arg(context->gamepath.c_str()));
        return false;
    }

    int libtasArch = extractBinaryType(context->libtaspath);
    if (libtasArch <= 0) {
        QMessageBox::critical(nullptr, "Error", QString("Could not determine arch of file %1").arg(context->libtaspath.c_str()));
        return false;
    }

    /* Checking that the game can be executed by the user */
    if (gameArch != BT_PE32 && gameArch != BT_PE32P && access(context->gamepath.c_str(), X_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("Game %1 is not executable by the user").arg(context->gamepath.c_str()));
        return false;
    }

    /* Check for a possible libtas alternate file */
    if (((gameArch == BT_ELF32) || (gameArch == BT_PE32)) && (libtasArch == BT_ELF64)) {
        std::string lib32path = context->libtaspath;
        std::string libname("libtas.so");
        size_t pos = context->libtaspath.find(libname);
        lib32path.replace(pos, libname.length(), "libtas32.so");

        /* Checking that libtas32.so exists */
        if (access(lib32path.c_str(), F_OK) == 0) {
            /* Just in case, check the arch */
            libtasArch = extractBinaryType(lib32path);
        }
    }

    /* Check for wine presence in case of Windows executables */
    if ((gameArch == BT_PE32) || (gameArch == BT_PE32P)) {
        std::string winename = "wine";
        if (gameArch == BT_PE32P)
            winename += "64";

        std::string cmd = "which ";
        cmd += winename;
        FILE *output = popen(cmd.c_str(), "r");
        if (output != NULL) {
            fseek(output, 0, SEEK_END);
            long ssize = ftell(output);
            if (ssize == 0) {
                QMessageBox::critical(nullptr, "Error", QString("Trying to execute a Windows executable, but %1 cannot be found").arg(winename.c_str()));
                pclose(output);
                return false;
            }
            pclose(output);
        }
        else {
            QMessageBox::critical(nullptr, "Error", QString("Coundn't popen to locate wine"));
            return false;
        }
    }

    /* Arithmetic on enums is ugly but much shorter */
    if (gameArch != libtasArch && ((gameArch-2) != libtasArch)) {
        QMessageBox::critical(nullptr, "Error", QString("libtas.so library was compiled for a %1-bit arch but %2 has a %3-bit arch").arg((libtasArch%2)?32:64).arg(context->gamepath.c_str()).arg((gameArch%2)?32:64));
        return false;
    }

    return true;
}
