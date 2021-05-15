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

#include "MemLayout.h"
#include <sstream>
#include <iostream>

MemLayout::MemLayout(pid_t p)
{
    pid = p;
    MemSection::reset();
}

MemLayout::~MemLayout()
{
    if (mapsfile)
        mapsfile.close();
}

void MemLayout::readLayout()
{
    /* Compose the filename for the /proc memory map, and open it. */
    std::ostringstream oss;
    oss << "/proc/" << pid << "/maps";
    mapsfile.open(oss.str());
    if (!mapsfile) {
        std::cerr << "Could not open " << oss.str() << std::endl;
        return;
    }
}

uint64_t MemLayout::totalSize(int type_flag)
{
    readLayout();
    
    std::string line;
    MemSection::reset();

    int total_size = 0;
    while (std::getline(mapsfile, line)) {
        MemSection section;
        section.readMap(line);

        if (section.type & type_flag) {
            total_size += section.size;
        }
    }
    
    if (mapsfile)
        mapsfile.close();
        
    return total_size;
}

bool MemLayout::nextSection(int type_flag, MemSection &section)
{
    if (!mapsfile) {
        readLayout();
        MemSection::reset();
    }

    if (!mapsfile) {
        section.addr = 0;
        return false;
    }
    
    std::string line;
    while (std::getline(mapsfile, line)) {
        section.readMap(line);

        if (section.type & type_flag) {
            return true;
        }
    }
    
    section.addr = 0;
    return false;
}