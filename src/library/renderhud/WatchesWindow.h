/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_IMGUI_WATCHESWINDOW_H_INCL
#define LIBTAS_IMGUI_WATCHESWINDOW_H_INCL

#include <string>

namespace libtas {

/**
 * @namespace WatchesWindow
 * @brief Helper namespace for memory watch in the HUD.
 */
namespace WatchesWindow
{
    /**
     * @brief Adds a new watch string to the window.
     *
     * @param[in] watch String to show
     */
    void insert(std::string watch);

    /**
     * @brief Clears all watch strings.
     */
    void reset();

    /**
     * @brief Draws the watch window.
     *
     * @param[in,out] p_open Controls whether the watch window is visible
     */
    void draw(bool* p_open);
}

}

#endif
