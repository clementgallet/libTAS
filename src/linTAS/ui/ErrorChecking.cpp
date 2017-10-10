/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ErrorChecking.h"
#include <FL/fl_ask.H>
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

    if (!checkArchType(context->gamepath, context->libtaspath))
        return false;

    return true;
}

bool ErrorChecking::checkGameExists(std::string gamepath)
{
    /* Checking that the game binary exists */
    if (access(gamepath.c_str(), F_OK) != 0) {
        fl_alert("Game path %s was not found", gamepath.c_str());
        return false;
    }

    /* Checking that the game can be executed by the user */
    if (access(gamepath.c_str(), X_OK) != 0) {
        fl_alert("Game %s is not executable by the user", gamepath.c_str());
        return false;
    }

    return true;
}

bool ErrorChecking::checkMovieExists(std::string moviepath)
{
    /* Checking that the movie file exists */
    if (access(moviepath.c_str(), F_OK) != 0) {
        fl_alert("Movie path %s was not found", moviepath.c_str());
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
        fl_alert("The movie path %s is not an absolute path", moviedir.c_str());
        return false;
    }

    /* Check that the directory of the moviefile exists */
    struct stat sb;
    if (stat(moviedir.c_str(), &sb) == -1) {
        if (errno == ENOENT) {
            /* The directory does not exist */
            fl_alert("The directory of the moviefile %s does not exists", moviedir.c_str());
            return false;
        }
        else {
            /* Another error */
            fl_alert("The directory of the moviefile %s cannot be accessed", moviedir.c_str());
            return false;
        }
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        fl_alert("The directory of the moviefile %s is not a directory", moviedir.c_str());
        return false;
    }

    /* Checking that the user can create a movie file */
    if (access(moviedir.c_str(), W_OK) != 0) {
        fl_alert("You don't have permission to create moviefile %s", moviepath.c_str());
        return false;
    }

    /* Prompt a confirmation message if overwriting a movie file */
    if (access(moviepath.c_str(), F_OK) == 0) {
        int choice = fl_choice("The movie file %s does exist. Do you want to overwrite it?", "Yes", "No", 0, moviepath.c_str());
        if (choice == 1)
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
    std::string cmd = "file -b '";
    cmd += path;
    cmd += "'";

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
        fl_alert("Could not call `file` command on %s", path.c_str());
        return -1;
    }

    if (outputstr.compare(0, 10, "ELF 64-bit") == 0) {
        return 64;
    }

    if (outputstr.compare(0, 10, "ELF 32-bit") == 0) {
        return 32;
    }

    fl_alert("Could not determine arch of file %s based on the `file` command output\nFull output: %s", path.c_str(), outputstr.c_str());
    return 0;
}

bool ErrorChecking::checkArchType(std::string gamepath, std::string libtaspath)
{
    /* Checking that the game binary exists (again) */
    if (access(gamepath.c_str(), F_OK) != 0) {
        fl_alert("Game path %s was not found", gamepath.c_str());
        return false;
    }

    /* Checking that the libtas.so library path is correct */
    if (access(libtaspath.c_str(), F_OK) != 0) {
        fl_alert("libtas.so library at %s was not found", libtaspath.c_str());
        return false;
    }

    int gameArch = extractFileArch(gamepath);
    int libtasArch = extractFileArch(libtaspath);

    if ((gameArch <= 0) || (libtasArch <= 0))
        /* Alert messages where already prompted in the extractFileArch function */
        return false;

    if (gameArch != libtasArch) {
        fl_alert("libtas.so library was compiled for a %d-bit arch but %s has a %d-bit arch", libtasArch, gamepath.c_str(), gameArch);
        return false;
    }

    return true;
}
