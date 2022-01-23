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

#include "MemSection.h"
#include <sstream>
#include <iostream>
#include <algorithm>

bool MemSection::heap_discovered = false;

void MemSection::reset()
{
    heap_discovered = false;
}

bool MemSection::followFlags(int flags)
{
    if ((flags & MemNoSpecial) && (type == MemSpecial))
        return false;

    if ((flags & MemNoRO) && (!writeflag))
        return false;

    if ((flags & MemNoExec) && (execflag))
        return false;
        
    return true;
}

void MemSection::readMap(std::string& line)
{
    std::istringstream iss(line);

    char d;
    /* Read addresses */
    iss >> std::hex >> addr >> d >> endaddr;
    size = endaddr - addr;

    /* Read and parse flags */
    std::string flags;
    iss >> flags;
    readflag = (flags.find('r') != std::string::npos);
    writeflag = (flags.find('w') != std::string::npos);
    execflag = (flags.find('x') != std::string::npos);
    sharedflag = (flags.find('s') != std::string::npos);

    /* Read the rest */
    iss >> offset >> device >> inode;

    /* Put the rest of the line into filename */
    std::getline(iss, filename);

    /* Trim filename */
    filename.erase(filename.begin(), std::find_if(filename.begin(), filename.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    filename.erase(std::find_if(filename.rbegin(), filename.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), filename.end());

    /* Determine section type */
    if (!readflag) {
        type = MemNoRead;
        return;
    }

    if (filename.compare("[vsyscall]") == 0 ||
        filename.compare("[vectors]") == 0 ||
        filename.compare("[vvar]") == 0 ||
        filename.compare("[vdso]") == 0) {
        type = MemSpecial;
        return;
    }

    if (filename.find("[stack") == 0) {
        type = MemStack;
        return;
    }

    if (filename.compare("[heap]") == 0) {
        type = MemHeap;
        heap_discovered = true;
        return;
    }

    bool isempty = (filename.find_first_not_of("\t\n ") == std::string::npos);

    if (heap_discovered) {
        if (!isempty) {
            if (writeflag) {
                type = MemFileMappingRW;
                return;
            }

            type = MemFileMappingRO;
            return;
        }

        if (writeflag) {
            type = MemAnonymousMappingRW;
            return;
        }

        type = MemAnonymousMappingRO;
        return;
    }

    if (isempty) {
        type = MemBSS;
        return;
    }

    if (writeflag) {
        type = MemDataRW;
        return;
    }

    if (execflag) {
        type = MemText;
        return;
    }

    type = MemDataRO;
}
