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

#include "WindowTitle.h"
#include <sstream>
#include "logging.h"
#include "../shared/SharedConfig.h"
#include "global.h" // game_info

namespace libtas {

static std::string orig_title;
static std::function<void(const char*)> set_title;

void WindowTitle::setOriginalTitle(const char* title)
{
    if (title)
        orig_title = title;
}

void WindowTitle::setUpdateFunc(std::function<void(const char*)> func)
{
    if (!set_title)
        set_title = func;
}

void WindowTitle::update(float fps, float lfps)
{
    if (!set_title)
        return;

    static float cur_fps, cur_lfps = 0;
    if (fps > 0) cur_fps = fps;
    if (lfps > 0) cur_lfps = lfps;

    std::ostringstream out;
    out << " (fps: " << std::fixed << std::setprecision(1) << cur_fps;
    out << " - lfps: " << cur_lfps << ") - status: ";
    if (shared_config.running)
        out << "running";
    else
        out << "paused";
    if (shared_config.fastforward)
        out << " fastforward";
    if (shared_config.av_dumping)
        out << " dumping";

    std::string new_title = orig_title + out.str();

    set_title(new_title.c_str());
}
}
