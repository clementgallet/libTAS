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

#ifndef LINTAS_CONFIG_H_INCLUDED
#define LINTAS_CONFIG_H_INCLUDED

#include <string>
#include <memory>
#include "../shared/SharedConfig.h"
#include "KeyMapping.h"
#include <FL/Fl_Preferences.H>

/* Structure holding program configuration that is saved in a file.
 * We use FLTK preferences system for that, which basically stores preferences
 * in a key/value way in a plain text file. Each game (determined by its
 * executable name) gets its own preferences.
 */

class Config {
private:
    std::unique_ptr<Fl_Preferences> prefs;

public:
    ~Config();

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

    /* Was the dump file modified */
    bool dumpfile_modified;

    /* Save the config into the config file */
    void save();

    /* Load a game-specific config from the config file */
    void load(const std::string& gamepath);

};

#endif
