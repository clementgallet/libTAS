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

#include "IOProcessDevice.h"
#include "MemAccess.h"
#include "MemSection.h"

#include <iostream>

IOProcessDevice::IOProcessDevice(QObject *parent) : QIODevice(parent) {
    mem_section.addr = 0;
    mem_section.size = 0;
}

void IOProcessDevice::setSection(const MemSection& section)
{
    mem_section = section;
}

qint64 IOProcessDevice::size() const
{
    return static_cast<qint64>(mem_section.size);
}

qint64 IOProcessDevice::readData(char *data, qint64 maxSize)
{
    size_t size = MemAccess::read(data, reinterpret_cast<void*>(mem_section.addr+offset), maxSize);
    if (size < 0)
        return -1;

    offset += size;
    return static_cast<qint64>(size);
}

qint64 IOProcessDevice::writeData(const char *data, qint64 maxSize)
{
    size_t size = MemAccess::write(const_cast<char *>(data), reinterpret_cast<void*>(mem_section.addr+offset), maxSize);
    if (size < 0)
        return -1;
    offset += size;
    return static_cast<qint64>(size);
}

bool IOProcessDevice::seek(qint64 pos)
{
    QIODevice::seek(pos);
    offset = pos;
    return true;
}

bool IOProcessDevice::isSequential() const
{
    return false;
}
