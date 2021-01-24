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

// #include <QApplication>

// #include "ui/MainWindow.h"

#include "config.h"
#include "GameThread.h"
#include "utils.h"
#include "../shared/SharedConfig.h"

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h> // chdir()
#include <fcntl.h> // O_RDWR, O_CREAT

#include <sys/personality.h>
#ifndef HAVE_PERSONALITY
# include <syscall.h>
# define personality(pers) ((long)syscall(SYS_personality, pers))
#endif

void GameThread::launch(Context *context)
{
    /* Update the LD_LIBRARY_PATH environment variable if the user set one */
    if (!context->config.libdir.empty()) {
        char* oldlibpath = getenv("LD_LIBRARY_PATH");
        std::string libpath = context->config.libdir;
        if (oldlibpath) {
            libpath.append(":");
            libpath.append(oldlibpath);
        }
        setenv("LD_LIBRARY_PATH", libpath.c_str(), 1);
    }

    /* Change the working directory to the user-defined one or game directory */
    std::string newdir = context->config.rundir;
    if (newdir.empty()) {
        /* Get the game directory from path */
        size_t sep = context->gamepath.find_last_of("/");
        if (sep != std::string::npos)
            newdir = context->gamepath.substr(0, sep);
        else
            newdir = ""; // Should not happen
    }

    if (0 != chdir(newdir.c_str())) {
        std::cerr << "Could not change the working directory to " << newdir << std::endl;
    }

    /* Set PWD environment variable because games may use it and chdir
     * does not update it.
     */
    setenv("PWD", newdir.c_str(), 1);

    /* Set where stderr of the game is redirected */
    int fd;
    std::string logfile = context->gamepath + ".log";
    switch(context->config.sc.logging_status) {
        case SharedConfig::NO_LOGGING:
            fd = open("/dev/null", O_RDWR, S_IRUSR | S_IWUSR);
            dup2(fd, 2);
            close(fd);
            break;
        case SharedConfig::LOGGING_TO_FILE:
            fd = open(logfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(fd, 2);
            close(fd);
            break;
        case SharedConfig::LOGGING_TO_CONSOLE:
        default:
            break;
    }

    /* Change settings based on game arch */
    int gameArch = extractBinaryType(context->gamepath);
    int libtasArch = extractBinaryType(context->libtaspath);

    /* Switch to libtas32.so if required */
    if (((gameArch == BT_ELF32) || (gameArch == BT_PE32)) && (libtasArch == BT_ELF64)) {
        std::string libname("libtas.so");
        size_t pos = context->libtaspath.find(libname);
        context->libtaspath.replace(pos, libname.length(), "libtas32.so");
        /* libtas32.so presence was already checked in ui/ErrorChecking.cpp */
        libtasArch = extractBinaryType(context->libtaspath);
    }

    /* Set additional environment variables regarding Mesa configuration */
    if (context->config.sc.opengl_soft)
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    else
        unsetenv("LIBGL_ALWAYS_SOFTWARE");

    /* Pass libtas library path to the game */
    setenv("LIBTAS_LIBRARY_PATH", context->libtaspath.c_str(), 1);

    setenv("LIBTAS_START_FRAME", std::to_string(context->framecount).c_str(), 1);


    /* Disable Address Space Layout Randomization for the game, so that ram
     * watch addresses do not change on game restart.
     * Source: https://stackoverflow.com/questions/5194666/disable-randomization-of-memory-addresses/30385370#30385370
     */
    personality(ADDR_NO_RANDOMIZE);

    /* Build the argument list to be fed to execv */
    std::list<std::string> arg_list;

    /* Detect Windows executables and launch wine */
    if ((gameArch == BT_PE32) || (gameArch == BT_PE32P)) {

        if (context->config.use_proton && !context->config.proton_path.empty()) {
            /* Change the executable to proton */
            std::string winepath = context->config.proton_path;
            winepath += "/dist/bin/wine";
            if (gameArch == BT_PE32P)
                winepath += "64";
            arg_list.push_back(winepath);

            /* Set the env variables needed by Proton */
            std::string winedllpath = context->config.proton_path;
            winedllpath += "/dist/lib64/wine:";
            winedllpath += context->config.proton_path;
            winedllpath += "/dist/lib/wine";
            setenv("WINEDLLPATH", winedllpath.c_str(), 1);

            char* oldlibpath = getenv("LD_LIBRARY_PATH");
            std::string libpath = context->config.proton_path;
            libpath += "/dist/lib64/:";
            libpath += context->config.proton_path;
            libpath += "/dist/lib/";
            if (oldlibpath) {
                libpath.append(":");
                libpath.append(oldlibpath);
            }
            setenv("LD_LIBRARY_PATH", libpath.c_str(), 1);

            std::string wineprefix = context->config.proton_path;
            wineprefix += "/dist/share/default_pfx/";
            setenv("WINEPREFIX", wineprefix.c_str(), 1);
        }
        else {
            /* Change the executable to wine */
            std::string winename = "wine";
            if (gameArch == BT_PE32P)
                winename += "64";

            /* wine[64] presence was already checked in ui/ErrorChecking.cpp */
            std::string cmd = "which ";
            cmd += winename;
            FILE *output = popen(cmd.c_str(), "r");
            if (output != NULL) {
                std::array<char,256> buf;
                if (fgets(buf.data(), buf.size(), output) != 0) {
                    std::string winepath = std::string(buf.data());
                    winepath.pop_back(); // remove trailing newline
                    arg_list.push_back(winepath);
                }
                pclose(output);
            }
        }

        /* We need to delay libtas hooking for wine process. */
        setenv("LIBTAS_DELAY_INIT", "1", 1);

        /* Push the game executable as the first command-line argument */
        /* Wine can fail if not specifying a Windows path */
        context->gamepath.insert(0, "Z:");
        arg_list.push_back(context->gamepath);
    }
    else {
        if (context->attach_gdb) {
            arg_list.push_back("/usr/bin/gdb");
            arg_list.push_back("-q");
            arg_list.push_back("-ex");

            /* LD_PRELOAD must be set inside a gdb command to be effective */
            std::string ldpreloadstr = "set exec-wrapper env 'LD_PRELOAD=";
            ldpreloadstr += context->libtaspath;
            if (!context->old_ld_preload.empty()) {
                ldpreloadstr += ":";
                ldpreloadstr += context->old_ld_preload;
            }
            ldpreloadstr += "'";
            arg_list.push_back(ldpreloadstr);

            /* We are using SIGSYS and SIGXFSZ for savestates, so don't
             * print and pause when one signal is sent */
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGSYS nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGXFSZ nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGUSR1 nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGUSR2 nostop noprint");
            /* The following signals are used a lot in some games */
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGPWR nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIGXCPU nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIG35 nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("handle SIG36 nostop noprint");
            arg_list.push_back("-ex");
            arg_list.push_back("run");
            arg_list.push_back("--args");
        }

        /* Tell SDL >= 2.0.2 to let us override functions even if it is statically linked.
         * Does not work for wine games, because our custom SDL functions don't
         * have the correct calling convention. */
        setenv("SDL_DYNAMIC_API", context->libtaspath.c_str(), 1);

        arg_list.push_back(context->gamepath);
    }

    /* Argument string for sh */
    std::ostringstream sharg;

    /* Prepend LD_PRELOAD */
    if (!(context->attach_gdb && ((gameArch == BT_ELF32) || (gameArch == BT_ELF64)))) {
        /* Set the LD_PRELOAD environment variable to inject our lib to the game */
        sharg << "LD_PRELOAD=";
        if (!context->old_ld_preload.empty()) {
            sharg << context->libtaspath << ":" << context->old_ld_preload << " ";
        }
        else {
            sharg << context->libtaspath << " ";
        }
    }

    /* Escape and concatenate arguments */
    for (std::string arg : arg_list) {
        /* Replace all occurrences of `'` with `'\'''` */
        const std::string escape_string = "'\\''";
        size_t pos = arg.find("'");
        while(pos != std::string::npos) {
            arg.replace(pos, 1, escape_string);
            pos = arg.find("'", pos + escape_string.size());
        }

        /* Add to the argument string with enclosed `'` and space */
        sharg << "'" << arg << "' ";
    }

    /* Append the game command-line arguments */
    sharg << context->config.gameargs;

    /* Run the actual game with sh, taking care of splitting arguments */
    execlp("sh", "sh", "-c", sharg.str().c_str(), nullptr);
}
