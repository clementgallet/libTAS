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

#include "RamWatchDetailedBuilder.h"

#include "IRamWatchDetailed.h"
#include "RamWatchDetailed.h"
#include "TypeIndex.h"

IRamWatchDetailed* RamWatchDetailedBuilder::new_watch(uintptr_t addr, int type)
{
    switch (type) {
        case RamUnsignedChar:
            return new RamWatchDetailed<unsigned char>(addr);
        case RamChar:
            return new RamWatchDetailed<char>(addr);
        case RamUnsignedShort:
            return new RamWatchDetailed<unsigned short>(addr);
        case RamShort:
            return new RamWatchDetailed<short>(addr);
        case RamUnsignedInt:
            return new RamWatchDetailed<unsigned int>(addr);
        case RamInt:
            return new RamWatchDetailed<int>(addr);
        case RamUnsignedLong:
            return new RamWatchDetailed<uint64_t>(addr);
        case RamLong:
            return new RamWatchDetailed<int64_t>(addr);
        case RamFloat:
            return new RamWatchDetailed<float>(addr);
        case RamDouble:
            return new RamWatchDetailed<double>(addr);
    }
    return nullptr;
}
