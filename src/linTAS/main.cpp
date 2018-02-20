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

#include <QApplication>

#include "ui/MainWindow.h"
#include "Context.h"
#include "utils.h" // create_dir

#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <signal.h> // kill
#include <xcb/xcb.h>
// #define explicit _explicit
// #include <xcb/xkb.h>
// #undef explicit
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>


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
    qRegisterMetaTypeStreamOperators<HotKey>("HotKey");
    qRegisterMetaTypeStreamOperators<SingleInput>("SingleInput");

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

    // xcb_xkb_use_extension_cookie_t cookie =
    //     xcb_xkb_use_extension(context.conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    // std::unique_ptr<xcb_xkb_use_extension_reply_t> reply(xcb_xkb_use_extension_reply(context.conn, cookie, NULL));
    // if (!reply || !reply->supported) {
    //     std::cerr << "xcb xkb not supported" << std::endl;
    // }
    //
    // xcb_generic_error_t *error;
    // xcb_xkb_per_client_flags_cookie_t pcf = xcb_xkb_per_client_flags(context.conn,
    //     XCB_XKB_ID_USE_CORE_KBD,
    //     XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    //     // XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    //     0,
    //     0,
    //     0,
    //     0);
    // std::unique_ptr<xcb_xkb_per_client_flags_reply_t> pcf_reply(xcb_xkb_per_client_flags_reply(context.conn, pcf, &error));
    // if (error) {
    // 	std::cerr << "failed to set XKB per-client flags, not using detectable repeat" << std::endl;
    // }


    /* Init keymapping. This uses the X connection to get the list of KeyCodes,
     * so it must be called after opening it.
     */
    context.config.km.init(context.conn);

    /* libTAS.so path */
    /* TODO: Not portable! */
    ssize_t count = readlink( "/proc/self/exe", buf, PATH_MAX );
    std::string binpath = std::string( buf, (count > 0) ? count : 0 );
    char* binpathptr = const_cast<char*>(binpath.c_str());
    context.libtaspath = dirname(binpathptr);
    context.libtaspath += "/libTAS.so";

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
    QApplication app(argc, argv);

    MainWindow mainWin(&context);
    mainWin.show();

    app.exec();

    context.config.save();
    
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
