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

#include "ErrorChecking.h"

#include "Context.h"
#include "utils.h"

#include <QtWidgets/QMessageBox>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

bool ErrorChecking::allChecks(Context* context)
{
    if (!checkGameExists(context->gamepath, context->interactive))
        return false;

    if (context->config.sc.recording == SharedConfig::RECORDING_READ)
        if (!checkMovieExists(context->config.moviefile, context->interactive))
            return false;

    if (context->config.sc.recording == SharedConfig::RECORDING_WRITE)
        if (!checkMovieWriteable(context->config.moviefile, context->interactive))
            return false;

    if (!checkArchType(context))
        return false;

    return true;
}

bool ErrorChecking::checkGameExists(std::filesystem::path gamepath, bool interactive)
{
    /* Checking that the game binary exists */
    if (!std::filesystem::exists(gamepath)) {
        critical(QString("Game path %1 was not found").arg(gamepath.c_str()), interactive);
        return false;
    }

    return true;
}

bool ErrorChecking::checkMovieExists(std::filesystem::path moviepath, bool interactive)
{
    /* Checking that the movie file exists */
    if (!std::filesystem::exists(moviepath)) {
        critical(QString("Movie path %1 was not found").arg(moviepath.c_str()), interactive);
        return false;
    }

    return true;
}

bool ErrorChecking::checkMovieWriteable(std::filesystem::path moviepath, bool interactive)
{
    /* Extract the movie directory */
    std::filesystem::path moviedir = moviepath.parent_path();
    if (moviedir.empty()) {
        critical(QString("The movie path %1 is not an absolute path").arg(moviedir.c_str()), interactive);
        return false;
    }

    /* Check that the directory of the moviefile exists */
    if (!std::filesystem::exists(moviedir)) {
        critical(QString("The directory of the moviefile %1 cannot be accessed").arg(moviedir.c_str()), interactive);
        return false;
    }

    /* Checking that the user can create a movie file */
    if (std::filesystem::perms::none == (std::filesystem::status(moviedir).permissions() & std::filesystem::perms::owner_write)) {
        critical(QString("You don't have permission to create moviefile %1").arg(moviepath.c_str()), interactive);
        return false;
    }

    /* Prompt a confirmation message if overwriting a movie file */
    if (interactive && std::filesystem::exists(moviepath)) {
        QMessageBox::StandardButton btn = QMessageBox::question(nullptr, "Movie overwrite", QString("The movie file %1 does exist. Do you want to overwrite it?").arg(moviepath.c_str()), QMessageBox::Ok | QMessageBox::Cancel);
        if (btn != QMessageBox::Ok)
            return false;
    }

    return true;
}

bool ErrorChecking::checkArchType(Context* context)
{
    /* Checking that the game binary exists (again) */
    if (!std::filesystem::exists(context->gamepath)) {
        critical(QString("Game path %1 was not found").arg(context->gamepath.c_str()), context->interactive);
        return false;
    }

    /* Checking that the libtas.so library path is correct */
    if (!std::filesystem::exists(context->libtaspath)) {
        critical(QString("libtas.so library at %1 was not found. Make sure that the libtas.so file is in the same directory as the libTAS executable").arg(context->libtaspath.c_str()), context->interactive);
        return false;
    }

    /* Checking the type of game binary */
    int gameArch = extractBinaryType(context->gamepath) & BT_TYPEMASK; // Remove the extra flags

    if (gameArch == BT_UNKNOWN) {
        critical(QString("Could not determine arch of file %1").arg(context->gamepath.c_str()), context->interactive);
        return false;
    }

    if (gameArch == BT_SH) {
        critical(QString("libTAS does not support launching a game from a script. Please specify the game binary file instead"), context->interactive);
        return false;
    }

    int libtasArch = extractBinaryType(context->libtaspath) & BT_TYPEMASK;
    if (libtasArch <= 0) {
        critical(QString("Could not determine arch of file %1").arg(context->libtaspath.c_str()), context->interactive);
        return false;
    }

    /* Checking that the game can be executed by the user, or try adding the flag.
     * This is not needed for wine games.
     * MacOS apps are directories, and thus executables */
    if (gameArch != BT_PE32 && gameArch != BT_PE32P && gameArch != BT_NE && access(context->gamepath.c_str(), X_OK) != 0) {

        std::filesystem::permissions(context->gamepath, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

        if (std::filesystem::perms::none == (std::filesystem::status(context->gamepath).permissions() & std::filesystem::perms::owner_exec)) {
           critical(QString("Game %1 is not executable by the user, could not add the executable flag").arg(context->gamepath.c_str()), context->interactive);
           return false;
        }
    }

    /* Check for a possible libtas alternate file */
    if (((gameArch == BT_ELF32) || (gameArch == BT_PE32)) && (libtasArch == BT_ELF64)) {
        if (context->libtas32path.empty()) {
            critical(QString("Trying to launch a 32-bit game with a 64-bit build of libTAS, but libTAS couldn't guess the path to libtas32.so. Use --libtas32-so-path (see --help)"), context->interactive);
            return false;
        } else if (access(context->libtas32path.c_str(), F_OK) != 0) {
            critical(QString("Trying to launch a 32-bit game with a 64-bit build of libTAS, but no libtas32.so library could be found at %1. Make sure that libTAS was built with dual-arch support and that the libtas32.so file is in the same directory as the libTAS executable").arg(context->libtas32path.c_str()), context->interactive);
            return false;
        } else {
            libtasArch = extractBinaryType(context->libtas32path) & BT_TYPEMASK;
        }
    }

    /* Check for wine presence in case of Windows executables */
    if ((gameArch == BT_PE32) || (gameArch == BT_PE32P) || (gameArch == BT_NE)) {
        std::string winename = "wine";
        if (gameArch == BT_PE32P)
            winename += "64";
            

        std::string cmd = "which ";
        cmd += winename;
        
        std::string winepath = queryCmd(cmd);
        if (winepath.empty()) {
            critical(QString("Trying to execute a Windows executable, but %1 cannot be found").arg(winename.c_str()), context->interactive);
            return false;
        }
    }

    /* Check for gdb presence in case of Start and attach gdb */
    if (context->attach_gdb) {
        std::string cmd;

        switch (context->config.debugger) {
        case Config::DEBUGGER_GDB:
            cmd = "which gdb";
            break;
        case Config::DEBUGGER_LLDB:
            cmd = "which lldb";
            break;
        case Config::DEBUGGER_STRACE:
            cmd = "which strace";
            break;
        }

        std::string gdbpath = queryCmd(cmd);
        if (gdbpath.empty()) {
            critical(QString("Trying to start a game with attached debugger, but debugger cannot be found"), context->interactive);
            return false;
        }
    }

    /* Check for arch mismatch */
    if ((((gameArch == BT_ELF32) || (gameArch == BT_PE32)) && (libtasArch == BT_ELF64)) ||
        (((gameArch == BT_ELF64) || (gameArch == BT_PE32P)) && (libtasArch == BT_ELF32))) {
        critical(QString("libtas.so library was compiled for a %1-bit arch but %2 has a %3-bit arch").arg((libtasArch%2)?32:64).arg(context->gamepath.c_str()).arg((gameArch%2)?32:64), context->interactive);
        return false;
    }
    
    return true;
}

void ErrorChecking::critical(QString str, bool interactive)
{
    if (interactive) {
        QMessageBox::critical(nullptr, "Error", str);
    }
    else {
        std::cerr << str.toStdString() << std::endl;
    }
}
