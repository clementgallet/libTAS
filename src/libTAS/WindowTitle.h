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

#ifndef LIBTAS_WINDOWTITLE_H_INCL
#define LIBTAS_WINDOWTITLE_H_INCL

#include <functional>

namespace libtas {
namespace WindowTitle {

/* Store the original game window title */
void setOriginalTitle(const char* title);

/* Store the function (const char* -> void) which updates the game window title */
void setUpdateFunc(std::function<void(const char*)> func);

/* Update the window title to display various information including fps/lfps
 * running state, dumping state, which is appended to the original title.
 * If fps/lfps is 0, then use the previous value.
 */
void update(float fps, float lfps);

}
}

#endif
