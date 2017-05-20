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
#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname

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
    std::cout << "  -L, --libpath PATH  Indicate a path to additional libraries the game" << std::endl;
    std::cout << "                      will want to import." << std::endl;
    std::cout << "  -R, --runpath PATH  From which directory the game must be launched." << std::endl;
    std::cout << "                      Set to the executable directory by default." << std::endl;
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
    while ((c = getopt (argc, argv, "+r:w:d:l:L:R:h")) != -1)
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
                    context.recording = (c == 'r')?Context::RECORDING_READ_WRITE:Context::RECORDING_WRITE;
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
            case 'R':
                /* run directory */
                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.rundir = abspath;
                }
                break;
            case 'L':
                /* Shared library directory */
                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.libdir = abspath;
                }
                break;
            case '?':
                std::cout << "Unknown option character" << std::endl;
            case 'h':
                print_usage();
                return 0;
            default:
                return 1;
        }

    /* libTAS.so path */
    /* TODO: Not portable! */
    ssize_t count = readlink( "/proc/self/exe", buf, PATH_MAX );
    std::string binpath = std::string( buf, (count > 0) ? count : 0 );
    char* binpathptr = const_cast<char*>(binpath.c_str());
    context.libtaspath = dirname(binpathptr);
    context.libtaspath += "/libTAS.so"; // Quite bad programming

    /* Game path */
    abspath = realpath(argv[optind], buf);
    if (abspath) {
        context.gamepath = abspath;
    }

    /* Init the a game-specific config, loading a pref file if any */
    if (!context.gamepath.empty())
        context.config.load(context.gamepath);

    /* Game arguments */
    for (int i = optind+1; i < argc; i++) {
        context.config.gameargs += argv[i];
        context.config.gameargs += " ";
    }

    /* Open connection with the server */
    context.display = XOpenDisplay(NULL);
    if (context.display == NULL)
    {
        // ui_print("Cannot open display\n");
        return 1;
    }

    /* Starts the user interface */
    MainWindow& ui = MainWindow::getInstance();
    ui.build(&context);

    /* Start the threaded environnment */
    Fl::lock();

    Fl::run();
    XCloseDisplay(context.display);
}
