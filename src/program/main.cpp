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

#include <QApplication>

#include "ui/MainWindow.h"
#include "Context.h"
#include "utils.h" // create_dir

#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <signal.h> // kill
#include <xcb/xcb.h>
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
#include <xcb/xcb_cursor.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <getopt.h>


#define SOCKET_FILENAME "/tmp/libTAS.socket"

// std::vector<std::string> shared_libs;
Context context;

static void print_usage(void)
{
    std::cout << "Usage: libTAS [options] game_executable_relative_path [game_cmdline_arguments]" << std::endl;
    std::cout << "Options are:" << std::endl;
    std::cout << "  -d, --dump FILE     Start a audio/video encode into the specified FILE" << std::endl;
    std::cout << "  -r, --read MOVIE    Play game inputs from MOVIE file" << std::endl;
    std::cout << "  -w, --write MOVIE   Record game inputs into the specified MOVIE file" << std::endl;
    std::cout << "  -h, --help          Show this message" << std::endl;
}

int main(int argc, char **argv)
{
#ifdef LIBTAS_INTERIM_COMMIT
    std::cout << "Interim commit " << LIBTAS_INTERIM_COMMIT << " built on " << LIBTAS_INTERIM_DATE << std::endl;
#endif

    qRegisterMetaTypeStreamOperators<HotKey>("HotKey");
    qRegisterMetaTypeStreamOperators<SingleInput>("SingleInput");

    /* Parsing arguments */
    int c;
    char buf[PATH_MAX];
    char* abspath;
    std::ofstream o;
    std::string moviefile;

    static struct option long_options[] =
    {
        {"read", required_argument, nullptr, 'r'},
        {"write", required_argument, nullptr, 'w'},
        {"dump", required_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };
    int option_index = 0;

    // std::string libname;
    while ((c = getopt_long (argc, argv, "+r:w:d:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r':
            case 'w':
                /* Record/Playback movie file */

                /* We must be sure that the file exists, otherwise the following call
                 * to realpath will fail. */
                o.open(optarg, std::ofstream::app);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    moviefile = abspath;
                    context.config.sc.recording = (c == 'r')?SharedConfig::RECORDING_READ:SharedConfig::RECORDING_WRITE;
                }
                break;
            case 'd':
                /* Dump video to file */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.dumpfile = abspath;
                    context.config.dumping = true;
                }
                break;
            case '?':
                std::cout << "Unknown option character" << std::endl;
                break;
            case 'h':
                print_usage();
                return 0;
            default:
                return -1;
        }
    }

    /* Game path */
    abspath = realpath(argv[optind], buf);
    if (abspath) {
        context.gamepath = abspath;
    }

    /* Game arguments */
    for (int i = optind+1; i < argc; i++) {
        context.config.gameargs += argv[i];
        context.config.gameargs += " ";
    }

    /* Open connection with the server */
    // XInitThreads();
    context.conn = xcb_connect(NULL,NULL);
    if (xcb_connection_has_error(context.conn))
    {
        std::cerr << "Cannot open display" << std::endl;
        return -1;
    }

    /* Open the xkb extension */
    xcb_xkb_use_extension_cookie_t cookie =
        xcb_xkb_use_extension(context.conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_use_extension_reply_t *reply = xcb_xkb_use_extension_reply(context.conn, cookie, NULL);
    if (!reply || !reply->supported) {
        std::cerr << "xcb xkb not supported" << std::endl;
    }
    free(reply);

    /* Enable detectable autorepeat. Otherwise, KeyRelease events are generated
     * for each key press, even without holding the key.
     */
    xcb_generic_error_t *error;
    xcb_xkb_per_client_flags_cookie_t pcf = xcb_xkb_per_client_flags(context.conn,
        XCB_XKB_ID_USE_CORE_KBD,
        XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
        XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
        0,
        0,
        0);
    xcb_xkb_per_client_flags_reply_t *pcf_reply = xcb_xkb_per_client_flags_reply(context.conn, pcf, &error);
    if (error) {
    	std::cerr << "failed to set XKB per-client flags, not using detectable repeat" << std::endl;
    }
    free(pcf_reply);

    /* Initialize mouse cursor.
     * Taken from <https://xcb.freedesktop.org/tutorial/mousecursors/>
     */
    xcb_cursor_context_t *ctx;
    xcb_screen_t *screen;
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(context.conn));
    screen = iter.data;
    if (xcb_cursor_context_new(context.conn, screen, &ctx) < 0) {
        std::cerr << "failed to initialize xcb cursor context" << std::endl;
    }

    context.crosshair_cursor = xcb_cursor_load_cursor(ctx, "crosshair");

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
    context.libtaspath += "/libtas.so";

    /* Create the working directories */
    char *path = getenv("XDG_CONFIG_HOME");
    if (path) {
        context.config.configdir = path;
    }
    else {
        context.config.configdir = getenv("HOME");
        context.config.configdir += "/.config";
    }
    context.config.configdir += "/libTAS";
    if (create_dir(context.config.configdir) < 0) {
        std::cerr << "Cannot create dir " << context.config.configdir << std::endl;
        return -1;
    }

    /* Now that we have the config dir, we load the game-specific config */
    context.config.load(context.gamepath);

    /* Overwrite the movie path if specified in commandline */
    if (! moviefile.empty()) {
        context.config.moviefile = moviefile;
    }

    /* If the config file set custom directories for the remaining working dir,
     * we create these directories (if not already created).
     * Otherwise, we set and create the default ones. */
    std::string data_dir;
    path = getenv("XDG_DATA_HOME");
    if (path) {
        data_dir = path;
    }
    else {
        data_dir = getenv("HOME");
        data_dir += "/.local/share";
    }
    data_dir += "/libTAS";

    if (create_dir(data_dir) < 0) {
        std::cerr << "Cannot create dir " << data_dir << std::endl;
        return -1;
    }

    if (context.config.steamuserdir.empty()) {
        context.config.steamuserdir = data_dir + "/steam";
    }
    if (create_dir(context.config.steamuserdir) < 0) {
        std::cerr << "Cannot create dir " << context.config.steamuserdir << std::endl;
        return -1;
    }

    if (context.config.tempmoviedir.empty()) {
        context.config.tempmoviedir = data_dir + "/movie";
    }
    if (create_dir(context.config.tempmoviedir) < 0) {
        std::cerr << "Cannot create dir " << context.config.tempmoviedir << std::endl;
        return -1;
    }

    if (context.config.savestatedir.empty()) {
        context.config.savestatedir = data_dir + "/states";
    }
    if (create_dir(context.config.savestatedir) < 0) {
        std::cerr << "Cannot create dir " << context.config.savestatedir << std::endl;
        return -1;
    }

    /* Check if incremental savestates is supported by checking the soft-dirty bit */

    int fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd != -1) {
        lseek(fd, static_cast<off_t>((reinterpret_cast<uintptr_t>(&context)/4096)*8), SEEK_SET);

        uint64_t page;
        int ret = ::read(fd, &page, 8);
        if (ret != -1) {
            context.is_soft_dirty = page & (0x1ull << 55);
        }
        close(fd);
    }

    /* Starts the user interface */
    QApplication app(argc, argv);

    QLocale::setDefault(QLocale("C"));
    std::locale::global(std::locale::classic());

    MainWindow mainWin(&context);
    mainWin.show();

    app.exec();

    context.config.save(context.gamepath);

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

    xcb_free_cursor (context.conn, context.crosshair_cursor);
    xcb_cursor_context_free(ctx);

    xcb_disconnect(context.conn);
    return 0;
}
