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

#ifndef LIBTAS_CONFIG_H_INCLUDED
#define LIBTAS_CONFIG_H_INCLUDED

#include <QString>
#include <string>
#include <memory>
#include <list>

#include "../shared/SharedConfig.h"
#include "KeyMapping.h"


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
    KeyMapping km;

    /* Arguments passed to the game */
    std::string gameargs;

    /* Absolute path of the movie file */
    std::string moviefile;

    /* Absolute path of the dump file */
    std::string dumpfile;

    /* ffmpeg options */
    std::string ffmpegoptions;

    /* Was the dump file modified */
    bool dumpfile_modified;

    /* Were we started up with the -d option? */
    bool dumping;

    /* Path of the libraries used by the game */
    std::string libdir;

    /* Path where the game needs to run */
    std::string rundir;

    /* Directory holding our config files */
    std::string configdir;

    /* Directory holding temporary files for building movies */
    std::string tempmoviedir;

    /* Directory holding savestates and savestate movies */
    std::string savestatedir;

    /* Directory holding savestates and savestate movies */
    std::string llvm_perf;

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
    std::list<std::string> recent_gamepaths;

    /* Do we restart the game when it exits? */
    bool auto_restart = false;

    /* Save the config into the config file */
    void save(const std::string& gamepath);

    /* Load a game-specific config from the config file */
    void load(const std::string& gamepath);

private:
    QString iniPath(const std::string& gamepath) const;
};

#endif
