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

#ifndef LIBTAS_UTILS_H_INCLUDED
#define LIBTAS_UTILS_H_INCLUDED

#include <string>

/* Forward declaration */
struct Context;

/* Get the file from path */
std::string fileFromPath(const std::string& path);

/* Get the directory from path */
std::string dirFromPath(const std::string& path);

/* Get the absolute path of a file even if it doesn't exist */
std::string realpath_nonexist(const std::string& path);

/* Create a directory if it does not exist already */
int create_dir(const std::string& path);

/* Remove savestate files */
void remove_savestates(Context* context);

/* List of error codes */
enum BinaryType {
    BT_UNKNOWN,
    BT_ELF32, // 32-bit ELF file
    BT_ELF64, // 64-bit ELF file
    BT_PE32, // Windows 32-bit executable
    BT_PE32P, // Windows 64-bit executable
    BT_NE, // Windows 16-bit executable
    BT_SH, // Shell script
    BT_MACOS32, // MacOS 32-bit executable
    BT_MACOS64, // MacOS 64-bit executable
    BT_MACOSUNI, // MacOS universal binary
    BT_TYPEMASK = 0xff,
    BT_MACOSAPP = 0x100, // MacOS .app executable format
    BT_PIEAPP = 0x200, // Position-independent executable
};

/* Run the `file` command from a shell and extract the output of the command. */
int extractBinaryType(std::string path);

/* Get the executable from MacOS .app directory. */
std::string extractMacOSExecutable(std::string path);

/* Get the result of a shell command */
std::string queryCmd(const std::string& cmd, int* status = nullptr);

/* Get the result of a shell command and returns pid */
std::string queryCmdPid(const char **command, pid_t* popen_pid);

#endif
