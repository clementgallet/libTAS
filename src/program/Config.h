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

#ifndef LIBTAS_CONFIG_H_INCLUDED
#define LIBTAS_CONFIG_H_INCLUDED

#include "../shared/SharedConfig.h"

#include <QtCore/QString>
#include <string>
#include <memory>
#include <list>
#include <filesystem>

/* Forward declaration */
class KeyMapping;

/* Structure holding program configuration that is saved in a file.
 * We use QtSettings class for that, which basically stores preferences
 * in a key/value way in a plain text file. Each game (determined by its
 * executable name) gets its own preferences.
 */

class Config {
public:
    /* Set of the config that is sent to the game */
    SharedConfig sc;

    /* Do we need to resend the config ?*/
    bool sc_modified = false;

    /* key mapping */
    KeyMapping* km;

    /* Arguments passed to the game */
    std::string gameargs;

    /* Absolute path of the movie file */
    std::filesystem::path moviefile;

    /* Absolute path of the dump file */
    std::filesystem::path dumpfile;

    /* ffmpeg options */
    std::string ffmpegoptions;

    /* Was the dump file modified */
    bool dumpfile_modified;

    /* Were we started up with the -d option? */
    bool dumping;

    /* Absolute path of the screenshot file */
    std::filesystem::path screenshotfile;

    /* Path of the libraries used by the game */
    std::filesystem::path libdir;

    /* Path where the game needs to run */
    std::filesystem::path rundir;

    /* Directory holding our config files */
    std::filesystem::path configdir;

    /* Directory holding our data files */
    std::filesystem::path datadir;

    /* Path for the Steam user data folder */
    std::filesystem::path steamuserdir;

    /* Directory holding temporary files for building movies */
    std::filesystem::path tempmoviedir;

    /* Directory holding savestates and savestate movies */
    std::filesystem::path savestatedir;

    /* Directory holding files storing ram search results */
    std::filesystem::path ramsearchdir;

    /* Directory holding extra i386 libs required by some games */
    std::filesystem::path extralib32dir;

    /* Directory holding extra amd64 libs required by some games */
    std::filesystem::path extralib64dir;

    /* Flags when end of movie */
    enum MovieEnd {
        MOVIEEND_READ = 0,
        MOVIEEND_WRITE = 1,
    };

    int on_movie_end = MOVIEEND_READ;

    /* Do we enable autosaving? */
    bool autosave = true;

    /* Minimum delay between two autosaves */
    double autosave_delay_sec = 300;

    /* Minimum number of frames between two autosaves */
    int autosave_frames = 1000;

    /* Maximum number of autosaves for one movie */
    int autosave_count = 20;

    /* List of recent existing gamepaths */
    std::list<std::filesystem::path> recent_gamepaths;

    /* List of recent command-line options */
    std::list<std::string> recent_args;

    /* Do we restart the game when it exits? */
    bool auto_restart = false;

    /* Warp the pointer at the center of the game screen after each frame */
    bool mouse_warp = false;

    /* Use proton to launch Windows executables */
    bool use_proton = false;

    /* Autoscroll in the input editor */
    bool editor_autoscroll = true;

    /* true if rewind seeks to previous frame, false if seeks to modified frame */
    bool editor_rewind_seek = false;

    /* Use fastforward to rewind or seek to a specific frame in the input editor */
    bool editor_rewind_fastforward = true;

    /* Show marker panel */
    bool editor_panel_marker = false;

    /* Pause and stop fastforward when reaching an input editor marker */
    bool editor_marker_pause = false;

    /* Move markers on frame addition or removal */
    bool editor_move_marker = false;

    /* Proton absolute path */
    std::filesystem::path proton_path;

    /* Strace events, passed as [-e expr] */
    std::string strace_events;

    /* Save the config into the config file */
    void save(const std::filesystem::path& gamepath);

    /* Load a game-specific config from the config file */
    void load(const std::filesystem::path& gamepath);

    /* Save current ffmpeg options as default */
    void saveDefaultFfmpeg(const std::filesystem::path& gamepath);

    enum Debugger {
        DEBUGGER_GDB = 0,
        DEBUGGER_LLDB = 1,
        DEBUGGER_STRACE = 2,
    };

#ifdef __unix__
    int debugger = DEBUGGER_GDB;
#elif defined(__APPLE__) && defined(__MACH__)
    int debugger = DEBUGGER_LLDB;
#endif

    /* Allow the program to download libraries that will help running some games.
     * (-1 for unset) */
    int allow_downloads = -1;

private:
    QString iniPath(const std::filesystem::path& gamepath) const;

    /* Set default paths and optionally create directories */
    void createDirectories();
};

#endif
