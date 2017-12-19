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

#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include "Context.h"
#include "ui/MainWindow.h"
#include "utils.h" // create_dir
#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <signal.h> // kill
#include <xcb/xcb.h>

#define SOCKET_FILENAME "/tmp/libTAS.socket"

// std::vector<std::string> shared_libs;
Context context;

static void print_usage(void)
{
    std::cout << "Usage: linTAS [options] game_executable_relative_path [game_cmdline_arguments]" << std::endl;
    std::cout << "Options are:" << std::endl;
    std::cout << "  -d, --dump FILE     Start a audio/video encode into the specified FILE" << std::endl;
    std::cout << "  -r, --read MOVIE    Play game inputs from MOVIE file" << std::endl;
    std::cout << "  -w, --write MOVIE   Record game inputs into the specified MOVIE file" << std::endl;
    // std::cout << "  -l, --lib     PATH  Manually import a library" << std::endl;
    std::cout << "  -h, --help          Show this message" << std::endl;
}

int main(int argc, char **argv)
{

    /* Parsing arguments */
    int c;
    char buf[PATH_MAX];
    char* abspath;
    std::ofstream o;
    // std::string libname;
    while ((c = getopt (argc, argv, "+r:w:d:l:h")) != -1)
        switch (c) {
            case 'r':
            case 'w':
                /* Record/Playback movie file */

                /* We must be sure that the file exists, otherwise the following call
                 * to realpath will fail. */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.moviefile = abspath;
                    context.config.sc.recording = (c == 'r')?SharedConfig::RECORDING_READ:SharedConfig::RECORDING_WRITE;
                }
                break;
            case 'd':
                /* Dump video to file */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.sc.av_dumping = true;
                    context.config.dumpfile = abspath;
                }
                break;
            // case 'l':
            //     /* Shared library */
            //     abspath = realpath(optarg, buf);
            //     if (abspath) {
            //         libname = abspath;
            //         shared_libs.push_back(libname);
            //     }
            //     break;
            case '?':
                std::cout << "Unknown option character" << std::endl;
            case 'h':
                print_usage();
                return 0;
            default:
                return -1;
        }

    /* Open connection with the server */
    // XInitThreads();
    context.conn = xcb_connect(NULL,NULL);
    if (xcb_connection_has_error(context.conn))
    {
        std::cerr << "Cannot open display" << std::endl;
        return -1;
    }

    /* Init keymapping. This uses the X connection to get the list of KeyCodes,
     * so it must be called after opening it.
     */
    context.config.km.init(context.conn);

    /* libTAS.so path, relative to the executable directory, because we added
     * $ORIGIN to the executable runpath.
     */
    context.libtaspath = "./libTAS.so"; // setting "libTAS.so" does not work, why !?

    /* Game path */
    abspath = realpath(argv[optind], buf);
    if (abspath) {
        context.gamepath = abspath;
    }

    /* Create the working directories */
    std::string base_dir = getenv("HOME");
    base_dir += "/.libtas";
    if (create_dir(base_dir) < 0) {
        std::cerr << "Cannot create dir " << base_dir << std::endl;
        return -1;
    }

    context.config.configdir = base_dir + "/config";
    if (create_dir(context.config.configdir) < 0) {
        std::cerr << "Cannot create dir " << context.config.configdir << std::endl;
        return -1;
    }

    /* Now that we have the config dir, we load the game-specific config */
    if (!context.gamepath.empty())
        context.config.load(context.gamepath);

    /* If the config file set custom directories for the remaining working dir,
     * we create these directories (if not already created).
     * Otherwise, we set and create the default ones. */
    if (context.config.tempmoviedir.empty()) {
        context.config.tempmoviedir = base_dir + "/movie";
    }
    if (create_dir(context.config.tempmoviedir) < 0) {
        std::cerr << "Cannot create dir " << context.config.tempmoviedir << std::endl;
        return -1;
    }

    if (context.config.savestatedir.empty()) {
        context.config.savestatedir = base_dir + "/states";
    }
    if (create_dir(context.config.savestatedir) < 0) {
        std::cerr << "Cannot create dir " << context.config.savestatedir << std::endl;
        return -1;
    }

    /* Game arguments */
    for (int i = optind+1; i < argc; i++) {
        context.config.gameargs += argv[i];
        context.config.gameargs += " ";
    }

    /* Starts the user interface */
    MainWindow& ui = MainWindow::getInstance();
    ui.build(&context);

    /* Start the threaded environnment */
    Fl::lock();

    Fl::run();

    /* Check if the game is still running and try to close it softly */
    if (context.status != Context::INACTIVE) {
        context.status = Context::QUITTING;
        if (!context.config.sc.running) {
            context.config.sc.running = true;
            context.config.sc_modified = true;
        }

        struct timespec tim = {0, 10000000L};
        for (int i=0; i<20; i++) {
            // context.config.sc.running = true;
            // context.config.sc_modified = true;
            if (context.status == Context::INACTIVE)
                break;
            nanosleep(&tim, NULL);
        }

        if (context.status != Context::INACTIVE) {
            std::cout << "Game is not responding, killing it" << std::endl;
            /* The game didn't close. Kill it */
            kill(context.game_pid, SIGKILL);
        }
    }

    xcb_disconnect(context.conn);
    return 0;
}
