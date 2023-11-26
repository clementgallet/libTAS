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

#include "GameThread.h"
#include "utils.h"
#include "Context.h"
#include "../shared/SharedConfig.h"

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h> // chdir()
#include <fcntl.h> // O_RDWR, O_CREAT

int GameThread::detect_arch(Context *context)
{
    /* Change settings based on game arch */
    int gameArch = extractBinaryType(context->gamepath);    
    int libtasArch = extractBinaryType(context->libtaspath);

    /* Switch to libtas32.so if required */
    if ((((gameArch&BT_TYPEMASK) == BT_ELF32) || ((gameArch&BT_TYPEMASK) == BT_PE32)) && (libtasArch == BT_ELF64)) {
        context->libtaspath = context->libtas32path;
        /* libtas32.so presence was already checked in ui/ErrorChecking.cpp */
        libtasArch = extractBinaryType(context->libtaspath);
    }

    return gameArch;
}

void GameThread::set_env_variables(Context *context, int gameArch)
{
    /* Not interested in macos flag */
    gameArch &= BT_TYPEMASK;

#ifdef __unix__
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
#endif

    /* Change the working directory to the user-defined one or game directory */
    std::string newdir = context->config.rundir;
    if (newdir.empty())
        newdir = dirFromPath(context->gamepath);

    if (0 != chdir(newdir.c_str())) {
        std::cerr << "Could not change the working directory to " << newdir << std::endl;
    }

    /* Set PWD environment variable because games may use it and chdir
     * does not update it.
     */
    setenv("PWD", newdir.c_str(), 1);

    /* Set additional environment variables regarding Mesa and VDPAU configurations */
    if (context->config.sc.opengl_soft) {
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        setenv("VDPAU_DRIVER", "va_gl", 1);
        setenv("VDPAU_QUIRKS", "AvoidVA", 1);
    }
    else {
        unsetenv("LIBGL_ALWAYS_SOFTWARE");        
        unsetenv("VDPAU_DRIVER");        
        unsetenv("VDPAU_QUIRKS");        
    }

    /* If we prefer OpenAL Soft, we'll want to force it to use either SDL2 or ALSA internally */
    if (context->config.sc.openal_soft) {
        setenv("ALSOFT_DRIVERS", "sdl2,alsa", 1);
    }
    else {
        unsetenv("ALSOFT_DRIVERS");
    }

    /* Pass libtas library path to the game */
    setenv("LIBTAS_LIBRARY_PATH", context->libtaspath.c_str(), 1);

    setenv("LIBTAS_START_FRAME", std::to_string(context->framecount).c_str(), 1);

    /* Override timezone for determinism */
    setenv("TZ", "UTC0", 1);

    /* Set wine-specific env variables */
    if ((gameArch == BT_PE32) || (gameArch == BT_PE32P)) {

        /* Set specific env variables for Proton */
        if (context->config.use_proton && !context->config.proton_path.empty()) {
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

        /* We need to delay libtas hooking for wine process. */
        setenv("LIBTAS_DELAY_INIT", "1", 1);
    }
    else {
        /* Tell SDL >= 2.0.2 to let us override functions even if it is statically linked.
         * Does not work for wine games, because our custom SDL functions don't
         * have the correct calling convention. */
        setenv("SDL_DYNAMIC_API", context->libtaspath.c_str(), 1);
    }
}

std::list<std::string> GameThread::build_arg_list(Context *context, int gameArch)
{
    bool macappflag = gameArch & BT_MACOSAPP;
    gameArch &= BT_TYPEMASK;

    /* Build the argument list to be fed to execv */
    std::list<std::string> arg_list;

    if ((gameArch == BT_PE32) || (gameArch == BT_PE32P)) {
        if (context->config.use_proton && !context->config.proton_path.empty()) {
            /* Change the executable to proton */
            std::string winepath = context->config.proton_path;
            winepath += "/dist/bin/wine";
            if (gameArch == BT_PE32P)
                winepath += "64";
            arg_list.push_back(winepath);
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

        /* Push the game executable as the first command-line argument */
        /* Wine can fail if not specifying a Windows path */
        context->gamepath.insert(0, "Z:");
        arg_list.push_back(context->gamepath);
    }
    else {
        if (context->attach_gdb) {
            std::string cmd;

            switch (context->config.debugger) {
            case Config::DEBUGGER_GDB:
                cmd = "which gdb";
                break;
            case Config::DEBUGGER_LLDB:
                cmd = "which lldb";
                break;
            }

            FILE *output = popen(cmd.c_str(), "r");
            std::array<char,256> buf;
            fgets(buf.data(), buf.size(), output);
            std::string dbgpath = std::string(buf.data());
            dbgpath.pop_back(); // remove trailing newline
            pclose(output);

            arg_list.push_back(dbgpath);

            /* Push debugger arguments */
            switch (context->config.debugger) {
            case Config::DEBUGGER_GDB: {
                arg_list.push_back("-q");
                arg_list.push_back("-ex");

                /* LD_PRELOAD must be set inside a gdb
                 * command to be effective */
                std::string ldpreloadstr = "set exec-wrapper env 'LD_PRELOAD=";
                ldpreloadstr += context->libtaspath;
                if (!context->old_ld_preload.empty()) {
                    ldpreloadstr += ":";
                    ldpreloadstr += context->old_ld_preload;
                }
                ldpreloadstr += "'";
                arg_list.push_back(ldpreloadstr);

                /* We are using SIGSYS and SIGXFSZ for savestates, so don't
                 * print and pause when one signal is sent *
                 * Signals SIGPWR SIGXCPU SIG35 and SIG36 are used a lot in some games */
                arg_list.push_back("-ex");
                arg_list.push_back("handle SIGSYS SIGXFSZ SIGUSR1 SIGUSR2 SIGPWR SIGXCPU SIG35 SIG36 nostop noprint");
                arg_list.push_back("-ex");
                arg_list.push_back("run");
                arg_list.push_back("--args");

                break;
            }
            case Config::DEBUGGER_LLDB: {
                arg_list.push_back("-o");

                /* LD_PRELOAD/DYLD_INSERT_LIBRARIES must be set inside an lldb
                 * command to be effective */
#ifdef __unix__
                std::string ldpreloadstr = "set se target.env-vars 'LD_PRELOAD=";
#elif defined(__APPLE__) && defined(__MACH__)
                std::string ldpreloadstr = "set se target.env-vars 'DYLD_INSERT_LIBRARIES=";
#endif
                ldpreloadstr += context->libtaspath;
                if (!context->old_ld_preload.empty()) {
                    ldpreloadstr += ":";
                    ldpreloadstr += context->old_ld_preload;
                }
                ldpreloadstr += "'";
                arg_list.push_back(ldpreloadstr);

#if defined(__APPLE__) && defined(__MACH__)
                arg_list.push_back("-o");
                arg_list.push_back("set se target.env-vars 'DYLD_FORCE_FLAT_NAMESPACE=1'");
#endif

                /* We are using SIGSYS and SIGXFSZ for savestates, so don't
                 * print and pause when one signal is sent */
                arg_list.push_back("-o");
                arg_list.push_back("run");
                /* Signal handling cannot be performed in llvm before the process has started */
//                arg_list.push_back("-o");
//                arg_list.push_back("process handle -n false -p false -s false SIGSYS SIGXFSZ SIGUSR1 SIGUSR2 SIGXCPU");
                arg_list.push_back("--");

                break;
            }
            }
        }

        /* If MacOS app, insert the real executable */
        if (macappflag) {
            arg_list.push_back(extractMacOSExecutable(context->gamepath));
        }
        else {
            arg_list.push_back(context->gamepath);
        }
    }

    return arg_list;
}

void GameThread::detect_game_libraries(Context *context)
{
    /* Build a command to parse the first missing library from the game executable,
     * look at game directory and sub-directories for it */
    std::ostringstream oss_ml;
    oss_ml << "ldd '" << context->gamepath;
    oss_ml << "' | awk '/ => not found/ { print $1 }' | head -1";
    
    FILE *output = popen(oss_ml.str().c_str(), "r");
    std::array<char,256> buf;
    buf[0] = '\0';
    fgets(buf.data(), buf.size(), output);
    std::string missing_lib = std::string(buf.data());
    pclose(output);

    if (missing_lib.empty()) return;
    
    missing_lib.pop_back(); // remove trailing newline

    std::cout << "Try to find the location of " << missing_lib << " among game files."<< std::endl;

    std::string gamedir = dirFromPath(context->gamepath);
    std::ostringstream oss_lp;
    oss_lp << "find '" << gamedir << "' -name " << missing_lib << " -type f -print -quit";

    output = popen(oss_lp.str().c_str(), "r");
    buf[0] = '\0';
    fgets(buf.data(), buf.size(), output);
    std::string found_lib = std::string(buf.data());
    pclose(output);
    
    if (!found_lib.empty()) {
        found_lib.pop_back(); // remove trailing newline

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
}

void GameThread::launch(Context *context)
{
    /* Detect the game executable arch and handle 32-bit game on 64-bit arch case */
    int gameArch = detect_arch(context);

    /* Set all environment variables */
    set_env_variables(context, gameArch);
    
#ifdef __unix__
    detect_game_libraries(context);
#endif

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
            std::cout << "Logging to file: " << logfile << std::endl;
            fd = open(logfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(fd, 2);
            close(fd);
            break;
        case SharedConfig::LOGGING_TO_CONSOLE:
        default:
            break;
    }

    /* Build the argument list for running the game */
    std::list<std::string> arg_list = build_arg_list(context, gameArch);

    /* Argument string for sh */
    std::ostringstream sharg;

    /* Prepend LD_PRELOAD/DYLD_INSERT_LIBRARIES */
    if (!(context->attach_gdb && (!(((gameArch&BT_TYPEMASK) == BT_PE32) || ((gameArch&BT_TYPEMASK) == BT_PE32P))))) {
        /* Set the LD_PRELOAD/DYLD_INSERT_LIBRARIES environment variable to
         * inject our lib to the game */
#ifdef __unix__
        sharg << "LD_PRELOAD=";
#elif defined(__APPLE__) && defined(__MACH__)
        sharg << "DYLD_INSERT_LIBRARIES=";
#endif
        if (!context->old_ld_preload.empty()) {
            sharg << context->libtaspath << ":" << context->old_ld_preload << " ";
        }
        else {
            sharg << context->libtaspath << " ";
        }
        
        /* We need to set DYLD_FORCE_FLAT_NAMESPACE so that we can hook into the game */
#if defined(__APPLE__) && defined(__MACH__)
        sharg << "DYLD_FORCE_FLAT_NAMESPACE=1 ";
#endif
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
