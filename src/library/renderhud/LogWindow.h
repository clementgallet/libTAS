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

#ifndef LIBTAS_IMGUI_LOGWINDOW_H_INCL
#define LIBTAS_IMGUI_LOGWINDOW_H_INCL

namespace libtas {

/**
 * @namespace LogWindow
 * @brief Helper namespace for the HUD log console.
 */
namespace LogWindow
{
    /**
     * @brief Clears all stored log messages.
     */
    void clear();

    /**
     * @brief Appends a log message to the HUD log buffer.
     *
     * @param[in] beg Pointer to the start of the message text
     * @param[in] end Pointer to the end of the message text
     * @param[in] newline true to append a newline after the message
     */
    void addLog(const char* beg, const char* end, bool newline);

    /**
     * @brief Draws the log window overlay.
     *
     * @param[in,out] p_open Controls whether the log window is visible
     */
    void draw(bool* p_open);

}

}

#endif
