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

#ifndef LIBTAS_SCREENSHOT_H_INCL
#define LIBTAS_SCREENSHOT_H_INCL

#include <string>

namespace libtas {
namespace Screenshot {

    /* List of error codes */
    enum Error {
        ESCREENSHOT_OK = 0,
        ESCREENSHOT_NOSCREEN = -1, // Screen Capture was not inited
        ESCREENSHOT_NOPIPE = -2, // Could not create a pipe to ffmpeg
    };

    /* Save the screenshot to file, `draw` indicates if the current frame is
     * a draw frame. */
    int save(const std::string& screenshotfile, bool draw);

};

}

#endif
