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

#ifndef LIBTAS_ERRORCHECKING_H_INCLUDED
#define LIBTAS_ERRORCHECKING_H_INCLUDED

#include <QString>
#include <string>
#include <filesystem>

/* Forward declaration */
struct Context;

namespace ErrorChecking {

    /* Perform all the checks above */
    bool allChecks(Context* context);

    /* Check if the game path is an existing file, and that this file is
     * executable by the user.
     */
    bool checkGameExists(std::filesystem::path gamepath, bool interactive);

    /* Check that the moviefile exists */
    bool checkMovieExists(std::filesystem::path moviepath, bool interactive);

    /* Check that the directory of the movie file exists, and that the
     * movie file can be written in its directory.
     */
    bool checkMovieWriteable(std::filesystem::path moviepath, bool interactive);

    /* Check if the libtas.so library and the game executable where compiled
     * for the same arch.
     */
    bool checkArchType(Context* context);

    /* Show an error box or error message on terminal */
    void critical(QString str, bool interactive);

}

#endif
