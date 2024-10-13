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

#ifndef LIBTAS_IOPROCESSDEVICE_H_INCLUDED
#define LIBTAS_IOPROCESSDEVICE_H_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <QtCore/QIODevice>

#include "MemSection.h"

/* Interface to read and write memory to the game process */
class IOProcessDevice : public QIODevice {
    Q_OBJECT
    
public:
    IOProcessDevice(QObject *parent);

    void setSection(const MemSection& section);
    
    qint64 size() const override;
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    bool seek(qint64 pos) override;
    bool isSequential() const override;
    
private:
    uintptr_t offset;
    MemSection mem_section;
};

#endif
