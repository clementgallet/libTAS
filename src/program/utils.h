/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_UTILS_H_INCLUDED
#define LIBTAS_UTILS_H_INCLUDED

#include <string>
#include "Context.h"

/* Get the file from path */
std::string fileFromPath(const std::string& path);

/* Get the absolute path of a file even if it doesn't exist */
std::string realpath_nonexist(const std::string& path);

/* Create a directory if it does not exist already */
int create_dir(const std::string& path);

/* Remove savestate files */
void remove_savestates(Context* context);

/* List of error codes */
enum BinaryType {
    BT_UNKNOWN,
    BT_ELF32,
    BT_ELF64,
    BT_PE32,
    BT_PE32P,
    BT_SH,
    BT_MACOS32,
    BT_MACOS64,
    BT_MACOSUNI,
};

/* Run the `file` command from a shell and extract the output of the command. */
int extractBinaryType(std::string path);

/* Get the executable from MacOS .app directory. */
std::string extractMacOSExecutable(std::string path);

#endif
