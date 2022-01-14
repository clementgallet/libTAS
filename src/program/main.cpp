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

//#include "config.h"
#include <QtWidgets/QApplication>

#include "ui/MainWindow.h"
#include "Context.h"
#include "utils.h" // create_dir
#include "lua/Main.h"
#include "KeyMapping.h"
#include "ramsearch/MemScanner.h"
#ifdef __unix__
#include "KeyMappingXcb.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "KeyMappingQuartz.h"
#endif

#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <signal.h> // kill
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h> // _NSGetExecutablePath
#endif

#ifdef __unix__
#include <xcb/xcb.h>
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
#endif

Context context;

static void print_usage(void)
{
    std::cout << "Usage: libTAS [options] game_executable_relative_path [game_cmdline_arguments]" << std::endl;
    std::cout << "Options are:" << std::endl;
    std::cout << "  -d, --dump FILE         Start a audio/video encode into the specified FILE" << std::endl;
    std::cout << "  -r, --read MOVIE        Play game inputs from MOVIE file" << std::endl;
    std::cout << "  -w, --write MOVIE       Record game inputs into the specified MOVIE file" << std::endl;
    std::cout << "  -n, --non-interactive   Don't offer any interactive choice, so that it can run headless" << std::endl;
    std::cout << "      --libtas-so-path    Path to libtas.so (equivalent to setting LIBTAS_SO_PATH)" << std::endl;
    std::cout << "      --libtas32-so-path  Path to libtas32.so (equivalent to setting LIBTAS32_SO_PATH)" << std::endl;
    std::cout << "  -h, --help              Show this message" << std::endl;
}

int main(int argc, char **argv)
{
#ifdef LIBTAS_INTERIM_COMMIT
    std::cout << "Interim commit " << LIBTAS_INTERIM_COMMIT;
#ifdef LIBTAS_INTERIM_DATE
    std::cout << " built on " << LIBTAS_INTERIM_DATE;
#endif
    std::cout << std::endl;
#endif

    qRegisterMetaTypeStreamOperators<HotKey>("HotKey");
    qRegisterMetaTypeStreamOperators<SingleInput>("SingleInput");

    /* Parsing arguments */
    int c;
    char buf[PATH_MAX];
    std::string abspath;
    std::ofstream o;
    std::string moviefile;
    std::string dumpfile;
    int recordingmode = SharedConfig::RECORDING_WRITE;

    static struct option long_options[] =
    {
        {"read", required_argument, nullptr, 'r'},
        {"write", required_argument, nullptr, 'w'},
        {"dump", required_argument, nullptr, 'd'},
        {"non-interactive", no_argument, nullptr, 'n'},
        {"libtas-so-path", required_argument, nullptr, 'p'},
        {"libtas32-so-path", required_argument, nullptr, 'P'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };
    int option_index = 0;

    // std::string libname;
    while ((c = getopt_long (argc, argv, "+r:w:d:nh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r':
            case 'w':
                /* Record/Playback movie file */
                abspath = realpath_nonexist(optarg);
                if (!abspath.empty()) {
                    moviefile = abspath;
                    recordingmode = (c == 'r')?SharedConfig::RECORDING_READ:SharedConfig::RECORDING_WRITE;
                }
                break;
            case 'd':
                /* Dump video to file */
                abspath = realpath_nonexist(optarg);
                if (!abspath.empty()) {
                    dumpfile = abspath;
                }
                break;
            case 'n':
                context.interactive = false;
                break;
            case 'p':
                abspath = realpath_nonexist(optarg);
                if (!abspath.empty()) {
                    context.libtaspath = abspath;
                }
                break;
            case 'P':
                abspath = realpath_nonexist(optarg);
                if (!abspath.empty()) {
                    context.libtas32path = abspath;
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
    if (argv[optind]) {
        abspath = realpath_nonexist(argv[optind]);
        if (!abspath.empty()) {
            context.gamepath = abspath;
        }
    }

    /* Game arguments */
    std::string gameargsoverride;
    if (optind + 1 < argc) {
        gameargsoverride = argv[optind + 1];
    }
    for (int i = optind+2; i < argc; i++) {
        gameargsoverride += " ";
        gameargsoverride += argv[i];
    }

#ifdef __unix__
    /* Open connection with the server */
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

    /* Init keymapping. This uses the X connection to get the list of KeyCodes,
     * so it must be called after opening it.
     */
    context.config.km = new KeyMappingXcb(context.conn);
#elif defined(__APPLE__) && defined(__MACH__)
    context.config.km = new KeyMappingQuartz(nullptr);
#endif

    /* libtas.so path */
    /* TODO: Not portable! */
    if (context.libtaspath.empty()) {
        char *libtaspath_from_env = getenv("LIBTAS_SO_PATH");
        if (libtaspath_from_env) {
            abspath = realpath_nonexist(libtaspath_from_env);
            if (!abspath.empty()) {
                context.libtaspath = abspath;
            }
        }
    }
    if (context.libtaspath.empty()) {
#ifdef __unix__
        ssize_t count = readlink( "/proc/self/exe", buf, PATH_MAX );
        std::string binpath = std::string( buf, (count > 0) ? count : 0 );
        char* binpathptr = const_cast<char*>(binpath.c_str());
        context.libtaspath = dirname(binpathptr);
        context.libtaspath += "/libtas.so";
#elif defined(__APPLE__) && defined(__MACH__)
        uint32_t size = 4096;
        if (_NSGetExecutablePath(buf, &size) == 0)
            context.libtaspath = dirname(buf);
        else {
            std::cerr << "Could not get path of libTAS executable" << std::endl;
            exit(0);
        }
        context.libtaspath += "/libtas.dylib";
#endif
    }

    /* libtas32.so path */
    if (context.libtas32path.empty()) {
        char *libtas32path_from_env = getenv("LIBTAS32_SO_PATH");
        if (libtas32path_from_env) {
            abspath = realpath_nonexist(libtas32path_from_env);
            if (!abspath.empty()) {
                context.libtas32path = abspath;
            }
        }
    }
    if (context.libtas32path.empty()) {
        std::string lib32path = context.libtaspath;
        std::string libname("libtas.so");
        size_t pos = context.libtaspath.find(libname);
        if (pos != std::string::npos) {
            lib32path.replace(pos, libname.length(), "libtas32.so");
            context.libtas32path = lib32path;
        }
    }

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
    if (! gameargsoverride.empty()) {
        context.config.gameargs = gameargsoverride;
    }

    /* Overwrite the movie path if specified in commandline */
    if (! moviefile.empty()) {
        context.config.moviefile = moviefile;
        context.config.sc.recording = recordingmode;
    }

    /* Overwrite the dump path if specified in commandline */
    if (! dumpfile.empty()) {
        context.config.dumpfile = dumpfile;
        context.config.dumping = true;
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

    if (context.config.ramsearchdir.empty()) {
        context.config.ramsearchdir = data_dir + "/ramsearch";
    }
    if (create_dir(context.config.ramsearchdir) < 0) {
        std::cerr << "Cannot create dir " << context.config.ramsearchdir << std::endl;
        return -1;
    }
    MemScanner::init(context.config.ramsearchdir);

    /* Store current content of LD_PRELOAD/DYLD_INSERT_LIBRARIES */

#ifdef __unix__
    char* old_preload = getenv("LD_PRELOAD");
#elif defined(__APPLE__) && defined(__MACH__)
    char* old_preload = getenv("DYLD_INSERT_LIBRARIES");
#endif

    if (old_preload) context.old_ld_preload = old_preload;

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

    /* Start the lua VM */
    Lua::Main::init(&context);

    /* Starts the user interface */
    QApplication app(argc, argv);

    QLocale::setDefault(QLocale("C"));
    std::locale::global(std::locale::classic());

    MainWindow mainWin(&context);
    mainWin.show();

    app.exec();

    context.config.save(context.gamepath);

    /* Stop the lua VM */
    Lua::Main::exit(&context);

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

#ifdef __unix__
    xcb_disconnect(context.conn);
#endif
    return 0;
}
