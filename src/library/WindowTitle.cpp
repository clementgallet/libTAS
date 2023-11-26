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

#include "WindowTitle.h"
#include "logging.h"
#include "global.h" // Global::game_info
#include "GlobalState.h"
#include "../shared/SharedConfig.h"

#include <sstream>
#include <math.h>

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
    set_title = func;
}

void WindowTitle::update(float fps, float lfps)
{
    if (!set_title)
        return;

    /* Updating the title is probably slow, so we only do it when parameters
     * have changed.
     */
    static float last_fps, last_lfps = 0;
    static bool last_running = false;
    static bool last_fastforward = false;
    static bool last_dumping = false;

    if (last_running == Global::shared_config.running
     && last_fastforward == Global::shared_config.fastforward
     && last_dumping == Global::shared_config.av_dumping
     && fabsf(last_fps-fps) < 0.1
     && fabsf(last_lfps-lfps) < 0.1)
        return;

    std::ostringstream out;
    out << " (fps: " << std::fixed << std::setprecision(1) << fps;
    out << " - lfps: " << lfps << ") - status: ";
    if (Global::shared_config.running)
        out << "running";
    else
        out << "paused";
    if (Global::shared_config.fastforward)
        out << " fastforward";
    if (Global::shared_config.av_dumping)
        out << " dumping";

    std::string new_title = orig_title + out.str();
    NATIVECALL(set_title(new_title.c_str()));

    /* Store updated values */
    last_running = Global::shared_config.running;
    last_fastforward = Global::shared_config.fastforward;
    last_dumping = Global::shared_config.av_dumping;
    last_fps = fps;
    last_lfps = lfps;
}
}
