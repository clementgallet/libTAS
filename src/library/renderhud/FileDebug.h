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

#ifndef LIBTAS_IMGUI_FILEDEBUG_H_INCL
#define LIBTAS_IMGUI_FILEDEBUG_H_INCL

#include <cstdint>

namespace libtas {

/**
 * @namespace FileDebug
 * @brief Helper namespace for file-processing HUD debug output.
 */
namespace FileDebug
{
    /**
     * @brief Updates file debug state for the current frame.
     *
     * @param[in] framecount Current frame index
     */
    void update(uint64_t framecount);
    
    /**
     * @brief Draws the file debug window.
     *
     * @param[in] framecount Current frame index
     * @param[in,out] p_open Controls whether the file debug window is visible
     */
    void draw(uint64_t framecount, bool* p_open);
}

}

#endif
