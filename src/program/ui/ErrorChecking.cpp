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

    /* Checking that the game can be executed by the user */
    if (access(gamepath.c_str(), X_OK) != 0) {
        QMessageBox::critical(nullptr, "Error", QString("Game %1 is not executable by the user").arg(gamepath.c_str()));
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

/* Run the `file` command from a shell and extract the output of the command.
 * Returns -1 if the command could not be executed.
 * Returns 32 if the beginning of the output is ELF 32-bit.
 * Returns 64 if the beginning of the output is ELF 64-bit.
 * Otherwise, returns 0
 */
static int extractFileArch(std::string path)
{
    std::string cmd = "file -b \"";
    cmd += path;
    cmd += "\"";

    std::string outputstr("");

    FILE *output = popen(cmd.c_str(), "r");
    if (output != NULL) {
        std::array<char,1000> buf;
        if (fgets(buf.data(), buf.size(), output) != 0) {
            outputstr = std::string(buf.data());
        }
        pclose(output);
    }

    if (outputstr.empty()) {
        QMessageBox::critical(nullptr, "Error", QString("Could not call `file` command on %1").arg(path.c_str()));
        return -1;
    }

    if (outputstr.compare(0, 10, "ELF 64-bit") == 0) {
        return 64;
    }

    if (outputstr.compare(0, 10, "ELF 32-bit") == 0) {
        return 32;
    }

    QMessageBox::critical(nullptr, "Error", QString("Could not determine arch of file %1 based on the `file` command output\nFull output: %2").arg(path.c_str()).arg(outputstr.c_str()));
    return 0;
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

    int gameArch = extractFileArch(context->gamepath);
    int libtasArch = extractFileArch(context->libtaspath);

    if ((gameArch <= 0) || (libtasArch <= 0))
        /* Alert messages where already prompted in the extractFileArch function */
        return false;
        
    /* Check for a possible libtas alternate file */
    if ((gameArch == 32) && (libtasArch == 64)) {
        std::string lib32path = context->libtaspath;
        std::string libname("libtas.so");
        size_t pos = context->libtaspath.find(libname);
        lib32path.replace(pos, libname.length(), "libtas32.so");
        
        /* Checking that libtas32.so exists */
        if (access(lib32path.c_str(), F_OK) == 0) {
            /* Replace libtas path with the 32-bit version */
            context->libtaspath = lib32path;
            
            /* Just in case, check the arch */
            libtasArch = extractFileArch(context->libtaspath);
        }        
    }
    
    if (gameArch != libtasArch) {
        QMessageBox::critical(nullptr, "Error", QString("libtas.so library was compiled for a %1-bit arch but %2 has a %3-bit arch").arg(libtasArch).arg(context->gamepath.c_str()).arg(gameArch));
        return false;
    }

    return true;
}
