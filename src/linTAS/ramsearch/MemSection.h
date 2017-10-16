/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_MEMSECTION_H_INCLUDED
#define LINTAS_MEMSECTION_H_INCLUDED

#include <string>

/* Store a section of the game memory */
class MemSection {
    public:

        enum MemType {
            MemNoRead = 0x0000,
            MemText = 0x0001,
            MemDataRO = 0x0002,
            MemDataRW = 0x0004,
            MemBSS = 0x0008,
            MemHeap = 0x0010,
            MemFileMapping = 0x0020,
            MemAnonymousMappingRO = 0x0040,
            MemAnonymousMappingRW = 0x0080,
            MemStack = 0x0100,
            MemSpecial = 0x0200
        };

        /* All information gather from a single line of /proc/pid/maps */
        uintptr_t addr;
        uintptr_t endaddr;
        intptr_t size;
        bool readflag;
        bool writeflag;
        bool execflag;
        bool sharedflag;
        int offset;
        std::string device;
        int inode;
        std::string filename;

        /* Memory section type determined with protections and filename */
        MemType type;
        static bool heap_discovered;

        /* Needs to be called before reading a maps file, so that type is
         * correctly detected.
         */
        static void reset();

        /* Parse a single line from the /proc/pid/maps file. */
        void readMap(std::string& line);
};

#endif
